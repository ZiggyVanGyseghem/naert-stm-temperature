#include "cmsis_os2.h"
#include "GUI.h"
#include "DIALOG.h"
#include "temp.h"
#include <stdio.h> 
#include "cmsis_vio.h" 

#define ID_PROGBAR_TC74    (GUI_ID_USER + 0x01)
#define ID_TEXT_TC74       (GUI_ID_USER + 0x03)
#define ID_TEXT_THRESH     (GUI_ID_USER + 0x06)

// The variables controlling the system
int auto_mode = 1;         
int manual_fan_state = 0;  
int fan_threshold = 25; 

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

__NO_RETURN static void GUIThread (void *argument) {
  (void)argument;

  vioInit(); 
  GUI_Init();           
  
  WM_HWIN hWin = CreateLogViewer();

  WM_HWIN hProgTC74 = WM_GetDialogItem(hWin, ID_PROGBAR_TC74);
  WM_HWIN hTextTC74 = WM_GetDialogItem(hWin, ID_TEXT_TC74);
  WM_HWIN hTextThresh = WM_GetDialogItem(hWin, ID_TEXT_THRESH);
  
  uint8_t temp = 0; 
  char buf[30]; 
  
  // --- ADDED: A counter to manage USB writing ---
  int usb_timer = 0; 

  while (1) {
    Temp_Read (&temp); 

    PROGBAR_SetValue(hProgTC74, temp);
    sprintf(buf, "%d C", temp);
    TEXT_SetText(hTextTC74, buf);

    sprintf(buf, "Start Fan at: %d C", fan_threshold);
    TEXT_SetText(hTextThresh, buf);

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

    // --- ADDED: USB Logging Logic  ---
    usb_timer++;
    if (usb_timer >= 50) { // 50 loops * 100ms = 5 seconds
        usb_timer = 0; // Reset the timer
        
        // Open the text file on the USB stick (U0:) in "Append" mode ("a")
        FILE *f = fopen("U0:\\templog.txt", "a");
        if (f != NULL) {
            // Write the current temperature and close the file
            fprintf(f, "Temperature: %d C\n", temp);
            fclose(f);
        }
    }

    osDelay(100); 
  }
}