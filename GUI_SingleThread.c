#include "cmsis_os2.h"
#include "GUI.h"
#include "DIALOG.h"
#include "temp.h"
#include <stdio.h> 
#include "cmsis_vio.h" 
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h" 
#include "rl_fs.h" 

#define ID_PROGBAR_TC74    (GUI_ID_USER + 0x01)
#define ID_PROGBAR_INT     (GUI_ID_USER + 0x02) 
#define ID_TEXT_TC74       (GUI_ID_USER + 0x03)
#define ID_TEXT_INT        (GUI_ID_USER + 0x04) 
#define ID_TEXT_THRESH     (GUI_ID_USER + 0x06)

// The variables controlling the system
int auto_mode = 1;         
int manual_fan_state = 0;  
int fan_threshold = 25; 
int right_bar_mode = 0; // 0 = Internal Diode, 1 = Gyroscope

extern WM_HWIN CreateLogViewer(void);

#define GUI_THREAD_STK_SZ    (4096U)
static void         GUIThread (void *argument);         
static osThreadId_t GUIThread_tid;                      
static uint64_t     GUIThread_stk[GUI_THREAD_STK_SZ/8]; 

static const osThreadAttr_t GUIThread_attr = {
  .stack_mem  = &GUIThread_stk[0],
  .stack_size = sizeof(GUIThread_stk),
  .priority   = osPriorityNormal 
};

int Init_GUIThread (void) {
  GUIThread_tid = osThreadNew(GUIThread, NULL, &GUIThread_attr);
  if (GUIThread_tid == NULL) return(-1);
  return(0);
}

// --- INTERNAL TEMPERATURE SENSOR ---
void Init_Internal_Temp(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    ADC->CCR |= ADC_CCR_TSVREFE;
    ADC1->SQR3 = 18; 
    ADC1->SMPR1 |= (7 << 24); 
    ADC1->CR2 |= ADC_CR2_ADON;
}

int Read_Internal_Temp(void) {
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    uint32_t adc_val = ADC1->DR;
    float voltage = ((float)adc_val * 3300.0f) / 4095.0f; 
    float temp = ((voltage - 760.0f) / 2.5f) + 25.0f;     
    return (int)temp;
}

// --- GYROSCOPE PLACEHOLDER ---
int Read_Gyro_Temp(void) {
    // If you have the BSP library, the code to read the L3GD20 goes here!
    // For now, returning a dummy value of 35C to prove the button works.
    return 35; 
}
// ------------------------------------------------------

__NO_RETURN static void GUIThread (void *argument) {
  (void)argument;

  finit("U0:");   
  fmount("U0:");  

  vioInit(); 
  GUI_Init();           
  
  WM_HWIN hWin = CreateLogViewer();
    
  Init_Internal_Temp(); 

  WM_HWIN hProgInt = WM_GetDialogItem(hWin, ID_PROGBAR_INT); 
  WM_HWIN hTextInt = WM_GetDialogItem(hWin, ID_TEXT_INT);    
  WM_HWIN hProgTC74 = WM_GetDialogItem(hWin, ID_PROGBAR_TC74);
  WM_HWIN hTextTC74 = WM_GetDialogItem(hWin, ID_TEXT_TC74);
  WM_HWIN hTextThresh = WM_GetDialogItem(hWin, ID_TEXT_THRESH);
  
  uint8_t temp = 0; 
  char buf[30]; 
  int usb_timer = 0; 

  while (1) {
    // 1. Always read the main TC74 sensor
    Temp_Read (&temp); 
    PROGBAR_SetValue(hProgTC74, temp);
    sprintf(buf, "%d C", temp);
    TEXT_SetText(hTextTC74, buf);

    // Update the slider text
    sprintf(buf, "Start Fan at: %d C", fan_threshold);
    TEXT_SetText(hTextThresh, buf);
        
    // 2. Decide what the RIGHT bar shows
    int right_val = 0;
    if (right_bar_mode == 0) {
        right_val = Read_Internal_Temp(); // Show the 67C Processor temp
    } else {
        right_val = Read_Gyro_Temp();     // Show the Gyro temp
    }
    PROGBAR_SetValue(hProgInt, right_val);     
    sprintf(buf, "%d C", right_val);           
    TEXT_SetText(hTextInt, buf);              
            
    // 3. FAN LOGIC (ALWAYS USES TC74 'temp'!)
    if (auto_mode == 1) {
        if (temp > fan_threshold) { 
            vioSetSignal(vioLED1, vioLEDon);
        } else {
            vioSetSignal(vioLED1, vioLEDoff);
        }
    } else {
        if (manual_fan_state == 1) { 
            vioSetSignal(vioLED1, vioLEDon);
        } else {
            vioSetSignal(vioLED1, vioLEDoff);
        }
    }

    GUI_TOUCH_Exec(); 
    GUI_Exec();         
    GUI_X_ExecIdle();   

    // 4. USB LOGGING (ALWAYS USES TC74 'temp'!)
    usb_timer++;
    if (usb_timer >= 50) { 
        usb_timer = 0; 
        FILE *f = fopen("U0:\\templog.txt", "a");
        if (f != NULL) {
            fprintf(f, "TC74 Temp: %d C\n", temp);
            fclose(f);
        }
    }

    osDelay(100); 
  }
}