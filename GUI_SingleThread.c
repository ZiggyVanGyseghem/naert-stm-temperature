#include "cmsis_os2.h"
#include "GUI.h"
#include "DIALOG.h"
#include "temp.h"

// Make sure this matches the ID from your GUI Builder!
#define ID_PROGBAR_0 (GUI_ID_USER + 0x01) 

extern WM_HWIN CreateLogViewer(void);

/*----------------------------------------------------------------------------
 * GUIThread: GUI Thread for Single-Task Execution Model
 *---------------------------------------------------------------------------*/
#define GUI_THREAD_STK_SZ    (4096U)

static void         GUIThread (void *argument);         /* thread function */
static osThreadId_t GUIThread_tid;                      /* thread id */
static uint64_t     GUIThread_stk[GUI_THREAD_STK_SZ/8]; /* thread stack */

static const osThreadAttr_t GUIThread_attr = {
  .stack_mem  = &GUIThread_stk[0],
  .stack_size = sizeof(GUIThread_stk),
  .priority   = osPriorityNormal 
};

int Init_GUIThread (void) {

  GUIThread_tid = osThreadNew(GUIThread, NULL, &GUIThread_attr);
  if (GUIThread_tid == NULL) {
    return(-1);
  }

  return(0);
}

__NO_RETURN static void GUIThread (void *argument) {
  (void)argument;

  GUI_Init();           /* Initialize the Graphics Component */
  
  WM_HWIN hWin = CreateLogViewer();

  // Get the handle for your new Progress Bar widget
  WM_HWIN hProgbar = WM_GetDialogItem(hWin, ID_PROGBAR_0);
  
  // Variable to hold the temperature 
  uint8_t temp = 0; 

  while (1) {
    // Read the I2C sensor to get the latest temperature 
   // Temp_Read (&temp); 

    // Update the Progress Bar directly with the raw temperature integer!
    PROGBAR_SetValue(hProgbar, temp);

    GUI_TOUCH_Exec(); 
    GUI_Exec();         /* Execute all GUI jobs ... Return 0 if nothing was done. */ 
    GUI_X_ExecIdle();   /* Nothing left to do for the moment ... Idle processing */
  }
}