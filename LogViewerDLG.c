#include "DIALOG.h"
#include <stdio.h>

// Define the ID numbers for ALL our widgets
#define ID_WINDOW_0        (GUI_ID_USER + 0x00)
#define ID_PROGBAR_TC74    (GUI_ID_USER + 0x01) // Left thermometer (External)
#define ID_PROGBAR_INT     (GUI_ID_USER + 0x02) // Right thermometer (Internal/Gyro)
#define ID_TEXT_TC74       (GUI_ID_USER + 0x03)
#define ID_TEXT_INT        (GUI_ID_USER + 0x04)
#define ID_SLIDER_THRESH   (GUI_ID_USER + 0x05)
#define ID_TEXT_THRESH     (GUI_ID_USER + 0x06)
#define ID_BUTTON_MANUAL   (GUI_ID_USER + 0x07)
#define ID_BUTTON_AUTO     (GUI_ID_USER + 0x08)

// The Complete Blueprint for the 240x320 screen
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  // Main Window Background
  { WINDOW_CreateIndirect, "Main Screen", ID_WINDOW_0, 0, 0, 240, 320, 0, 0x0, 0 },
  
  // --- LEFT SIDE: TC74 Sensor ---
  { TEXT_CreateIndirect, "22C", ID_TEXT_TC74, 30, 40, 60, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "ProgbarTC74", ID_PROGBAR_TC74, 40, 65, 40, 110, 0, 0x0, 0 }, 
  
  // --- RIGHT SIDE: Internal Sensor / Gyro ---
  { TEXT_CreateIndirect, "20C", ID_TEXT_INT, 150, 40, 60, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "ProgbarInt", ID_PROGBAR_INT, 160, 65, 40, 110, 0, 0x0, 0 }, 

  // --- BOTTOM CONTROLS: Slider and Buttons ---
  { TEXT_CreateIndirect, "Start Fan at: 25 C", ID_TEXT_THRESH, 40, 190, 160, 20, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_THRESH, 20, 210, 200, 30, 0, 0x0, 0 },
  
  { BUTTON_CreateIndirect, "Manual", ID_BUTTON_MANUAL, 20, 260, 90, 40, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Automatic", ID_BUTTON_AUTO, 130, 260, 90, 40, 0, 0x0, 0 }
};

// Paint and configure the widgets when the screen loads
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    // 1. White Background
    WINDOW_SetBkColor(pMsg->hWin, GUI_WHITE);
    
    // 2. Setup LEFT Thermometer (TC74)
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_TC74);
    PROGBAR_SetMinMax(hItem, 0, 50); 
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); 
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); 
    
    // 3. Setup RIGHT Thermometer (Internal)
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_INT);
    PROGBAR_SetMinMax(hItem, 0, 50); 
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); 
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); 
    
    // 4. Setup the Slider (Range 0 to 50, default to 25)
    hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_THRESH);
    SLIDER_SetRange(hItem, 0, 50); 
    SLIDER_SetValue(hItem, 25);    
    
    // 5. Setup Fonts and text alignment
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TC74);
    TEXT_SetFont(hItem, GUI_FONT_16B_1);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER);

    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INT);
    TEXT_SetFont(hItem, GUI_FONT_16B_1);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER);

    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_THRESH);
    TEXT_SetFont(hItem, GUI_FONT_16B_1);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER);
    break;
    
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

// Function called by GUI_SingleThread.c to spawn this specific window
WM_HWIN CreateLogViewer(void);
WM_HWIN CreateLogViewer(void) {
  WM_HWIN hWin;
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}