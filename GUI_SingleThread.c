
#include "cmsis_os2.h"       // CMSIS-RTOS API (Real-Time Operating System)
#include "GUI.h"             // emWin Graphic Library core
#include "DIALOG.h"          // emWin Dialog Box API
#include "temp.h"            // I2C driver for the external TC74 sensor
#include <stdio.h>           // Standard I/O (for sprintf, fopen, fprintf)
#include "cmsis_vio.h"       // Virtual I/O (used to control the onboard LEDs)
#include "stm32f4xx_hal.h"   // Hardware Abstraction Layer
#include "stm32f4xx.h"       // Direct hardware register definitions
#include "rl_fs.h"           // Keil File System (for the FAT32 USB stick)

// --- GUI Widget IDs ---
// These match the IDs defined in WindowDLG.c so this file can find them.
#define ID_PROGBAR_TC74    (GUI_ID_USER + 0x01)
#define ID_PROGBAR_INT     (GUI_ID_USER + 0x02) 
#define ID_TEXT_TC74       (GUI_ID_USER + 0x03)
#define ID_TEXT_INT        (GUI_ID_USER + 0x04) 
#define ID_TEXT_THRESH     (GUI_ID_USER + 0x06)

// --- Global State Variables ---
// These control the state machine of the system and are modified by the GUI.
int auto_mode = 1;         // 1 = Auto Fan Mode, 0 = Manual Mode
int manual_fan_state = 0;  // 1 = Fan ON, 0 = Fan OFF (only used in Manual)
int fan_threshold = 25;    // Threshold temperature to trigger the fan in Auto
int right_bar_mode = 0;    // 0 = View Internal Diode, 1 = View Gyroscope placeholder

extern WM_HWIN CreateLogViewer(void); // External function to draw the GUI

// --- RTOS Thread Configuration ---
#define GUI_THREAD_STK_SZ    (4096U) // Stack size for the GUI Thread
static void         GUIThread (void *argument);         
static osThreadId_t GUIThread_tid;                      
static uint64_t     GUIThread_stk[GUI_THREAD_STK_SZ/8]; 

static const osThreadAttr_t GUIThread_attr = {
  .stack_mem  = &GUIThread_stk[0],
  .stack_size = sizeof(GUIThread_stk),
  .priority   = osPriorityNormal 
};

// Initializes the thread and hands it over to the RTOS scheduler
int Init_GUIThread (void) {
  GUIThread_tid = osThreadNew(GUIThread, NULL, &GUIThread_attr);
  if (GUIThread_tid == NULL) return(-1);
  return(0);
}

// =========================================================================
// HARDWARE ACCESS: INTERNAL TEMPERATURE SENSOR (ADC)
// Bypasses HAL libraries to use direct register manipulation (Bare-Metal)
// =========================================================================
void Init_Internal_Temp(void) {
    // 1. Enable the clock for ADC1
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    
    // 2. Wake up the internal temperature sensor (Temperature Sensor VREF Enable)
    ADC->CCR |= ADC_CCR_TSVREFE;
    
    // 3. Set the 1st conversion in the regular sequence to Channel 18 (Internal Temp)
    ADC1->SQR3 = 18; 
    
    // 4. Set maximum sample time for Channel 18 to ensure a stable reading
    ADC1->SMPR1 |= (7 << 24); 
    
    // 5. Turn on the ADC (A/D Converter ON)
    ADC1->CR2 |= ADC_CR2_ADON;
}

int Read_Internal_Temp(void) {
    // 1. Start the software conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;
    
    // 2. Wait in a blocking loop until End Of Conversion (EOC) flag is set
    while (!(ADC1->SR & ADC_SR_EOC));
    
    // 3. Read the raw digital value from the Data Register
    uint32_t adc_val = ADC1->DR;
    
    // 4. Convert raw ADC value to voltage (Assuming 3.3V reference)
    float voltage = ((float)adc_val * 3300.0f) / 4095.0f; 
    
    // 5. Apply the STM32 datasheet formula to find the actual Celsius temperature
    // Temp = (V_sense - V_25) / Avg_Slope + 25 
    float temp = ((voltage - 760.0f) / 2.5f) + 25.0f;     
    
    return (int)temp;
}

// --- GYROSCOPE PLACEHOLDER ---
int Read_Gyro_Temp(void) {
    // Because Keil is missing the L3GD20 SPI driver files (stm32f429i_discovery_gyroscope.h),
    // this acts as a placeholder to prove the UI state-machine toggle works.
    return 35; 
}
// =========================================================================

__NO_RETURN static void GUIThread (void *argument) {
  (void)argument;

  // 1. Initialize and Mount the USB FAT32 File System (Crucial for fopen)
  finit("U0:");   
  fmount("U0:");  

  // 2. Initialize Hardware & UI
  vioInit();            // Virtual I/O (LEDs)
  GUI_Init();           // emWin Graphics Engine
  
  WM_HWIN hWin = CreateLogViewer(); // Draw the screen
    
  Init_Internal_Temp(); // Boot up the internal ADC hardware

  // 3. Link C variables to the drawn GUI widgets
  WM_HWIN hProgInt = WM_GetDialogItem(hWin, ID_PROGBAR_INT); 
  WM_HWIN hTextInt = WM_GetDialogItem(hWin, ID_TEXT_INT);    
  WM_HWIN hProgTC74 = WM_GetDialogItem(hWin, ID_PROGBAR_TC74);
  WM_HWIN hTextTC74 = WM_GetDialogItem(hWin, ID_TEXT_TC74);
  WM_HWIN hTextThresh = WM_GetDialogItem(hWin, ID_TEXT_THRESH);
  
  uint8_t temp = 0;   // Holds the TC74 external temperature
  char buf[30];       // String buffer for screen text formatting
  int usb_timer = 0;  // Counter used to throttle USB writes

  // =========================================================================
  // MAIN INFINITE THREAD LOOP (Executes every 100ms)
  // =========================================================================
  while (1) {
      
    // --- 1. TC74 EXTERNAL SENSOR (LEFT BAR) ---
    Temp_Read (&temp); // Read from I2C
    PROGBAR_SetValue(hProgTC74, temp);
    sprintf(buf, "%d C", temp);
    TEXT_SetText(hTextTC74, buf);

    // Update the threshold text dynamically based on the slider value
    sprintf(buf, "Start Fan at: %d C", fan_threshold);
    TEXT_SetText(hTextThresh, buf);
        
    // --- 2. INTERNAL/GYRO SENSORS (RIGHT BAR) ---
    // Decides what the right bar shows based on the UI button state
    int right_val = 0;
    if (right_bar_mode == 0) {
        right_val = Read_Internal_Temp(); // Mode 0: Show the 67C Processor temp
    } else {
        right_val = Read_Gyro_Temp();     // Mode 1: Show the Gyro temp (35C)
    }
    PROGBAR_SetValue(hProgInt, right_val);     
    sprintf(buf, "%d C", right_val);           
    TEXT_SetText(hTextInt, buf);              
            
    // --- 3. FAN LOGIC (LED CONTROL) ---
    // Note: We use vioLED1 to avoid conflicts with the background "Blinky" thread
    if (auto_mode == 1) {
        if (temp > fan_threshold) { // Auto mode listens to the TC74 sensor
            vioSetSignal(vioLED1, vioLEDon);
        } else {
            vioSetSignal(vioLED1, vioLEDoff);
        }
    } else {
        if (manual_fan_state == 1) { // Manual mode only listens to the UI button
            vioSetSignal(vioLED1, vioLEDon);
        } else {
            vioSetSignal(vioLED1, vioLEDoff);
        }
    }

    // --- 4. EXECUTE GUI TASKS ---
    GUI_TOUCH_Exec();   // Check for screen taps
    GUI_Exec();         // Redraw changed widgets
    GUI_X_ExecIdle();   // Let the GUI sleep if nothing changed

    // --- 5. USB LOGGING LOGIC ---
    usb_timer++; // Increment by 1 every 100ms
    if (usb_timer >= 50) { // 50 * 100ms = 5000ms (5 seconds)
        usb_timer = 0; // Reset timer
        
        // Open the text file on the USB stick in "Append" mode
        FILE *f = fopen("U0:\\templog.txt", "a");
        if (f != NULL) {
            fprintf(f, "TC74 Temp: %d C\n", temp);
            fclose(f); // Crucial: Always close to save to flash memory
        }
    }

    // --- 6. RTOS DELAY ---
    // Puts this thread to sleep for 100 system ticks (100ms) to save CPU power
    osDelay(100); 
  }
}