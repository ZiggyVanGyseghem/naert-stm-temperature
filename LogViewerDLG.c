#include "DIALOG.h"

#define ID_WINDOW_0        (GUI_ID_USER + 0x00)
#define ID_PROGBAR_TC74    (GUI_ID_USER + 0x01) 
#define ID_PROGBAR_INT     (GUI_ID_USER + 0x02) 
#define ID_TEXT_TC74       (GUI_ID_USER + 0x03)
#define ID_TEXT_INT        (GUI_ID_USER + 0x04)
#define ID_SLIDER_THRESH   (GUI_ID_USER + 0x05)
#define ID_TEXT_THRESH     (GUI_ID_USER + 0x06)
#define ID_BUTTON_MANUAL   (GUI_ID_USER + 0x07)
#define ID_BUTTON_AUTO     (GUI_ID_USER + 0x08)
#define ID_BUTTON_SENSOR   (GUI_ID_USER + 0x09) 

extern int auto_mode;
extern int manual_fan_state;
extern int fan_threshold; 
extern int right_bar_mode; // 0 = Internal, 1 = Gyroscope

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Main Screen", ID_WINDOW_0, 0, 0, 240, 320, 0, 0x0, 0 },
  
  // Left Side (Always TC74)
  { TEXT_CreateIndirect, "22C", ID_TEXT_TC74, 30, 40, 60, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "ProgbarTC74", ID_PROGBAR_TC74, 40, 65, 40, 110, PROGBAR_CF_VERTICAL, 0x0, 0 }, 
  
  // Right Side (Internal or Gyro)
  { TEXT_CreateIndirect, "20C", ID_TEXT_INT, 150, 40, 60, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "ProgbarInt", ID_PROGBAR_INT, 160, 65, 40, 110, PROGBAR_CF_VERTICAL, 0x0, 0 }, 

  // --- MOVED: The button is now neatly under the right bar ---
  { BUTTON_CreateIndirect, "View: INT", ID_BUTTON_SENSOR, 140, 185, 80, 25, 0, 0x0, 0 },

  // Bottom Controls
  { TEXT_CreateIndirect, "Start Fan at: 25 C", ID_TEXT_THRESH, 40, 220, 160, 20, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_THRESH, 20, 240, 200, 30, 0, 0x0, 0 },
  
  { BUTTON_CreateIndirect, "Manual", ID_BUTTON_MANUAL, 20, 280, 90, 35, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Automatic", ID_BUTTON_AUTO, 130, 280, 90, 35, 0, 0x0, 0 }
};

static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int NCode;
  int Id;
  
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG: 
    WINDOW_SetBkColor(pMsg->hWin, GUI_WHITE);
    
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_TC74);
    PROGBAR_SetSkinClassic(hItem); 
    PROGBAR_SetMinMax(hItem, 0, 50); 
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); 
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); 
    
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_INT);
    PROGBAR_SetSkinClassic(hItem); 
    PROGBAR_SetMinMax(hItem, 0, 100); 
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); 
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); 
    
    hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_THRESH);
    SLIDER_SetRange(hItem, 0, 50); 
    SLIDER_SetValue(hItem, 25);    
    
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

  case WM_NOTIFY_PARENT: 
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    
    switch(Id) {
    case ID_SLIDER_THRESH:
      if (NCode == WM_NOTIFICATION_VALUE_CHANGED) {
         fan_threshold = SLIDER_GetValue(pMsg->hWinSrc);
      }
      break;

    // --- Switch between Internal and Gyro ---
    case ID_BUTTON_SENSOR:
      if (NCode == WM_NOTIFICATION_RELEASED) {
        if (right_bar_mode == 0) {
            right_bar_mode = 1; 
            BUTTON_SetText(pMsg->hWinSrc, "View: GYRO");
        } else {
            right_bar_mode = 0; 
            BUTTON_SetText(pMsg->hWinSrc, "View: INT");
        }
      }
      break;

    case ID_BUTTON_AUTO: 
      if (NCode == WM_NOTIFICATION_RELEASED) auto_mode = 1; 
      break;
      
    case ID_BUTTON_MANUAL: 
      if (NCode == WM_NOTIFICATION_RELEASED) {
        auto_mode = 0; 
        if (manual_fan_state == 0) manual_fan_state = 1; 
        else manual_fan_state = 0;                       
      }
      break;
    }
    break;
    
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

WM_HWIN CreateLogViewer(void);
WM_HWIN CreateLogViewer(void) {
  return GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}