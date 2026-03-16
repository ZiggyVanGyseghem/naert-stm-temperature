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

// Link variables from GUI_SingleThread
extern int auto_mode;
extern int manual_fan_state;
extern int fan_threshold; 

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Main Screen", ID_WINDOW_0, 0, 0, 240, 320, 0, 0x0, 0 },
  
  { TEXT_CreateIndirect, "22C", ID_TEXT_TC74, 30, 40, 60, 20, 0, 0x0, 0 },
  // ADDED PROGBAR_CF_VERTICAL to force it up and down!
  { PROGBAR_CreateIndirect, "ProgbarTC74", ID_PROGBAR_TC74, 40, 65, 40, 110, PROGBAR_CF_VERTICAL, 0x0, 0 }, 
  
  { TEXT_CreateIndirect, "20C", ID_TEXT_INT, 150, 40, 60, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "ProgbarInt", ID_PROGBAR_INT, 160, 65, 40, 110, PROGBAR_CF_VERTICAL, 0x0, 0 }, 

  { TEXT_CreateIndirect, "Start Fan at: 25 C", ID_TEXT_THRESH, 40, 190, 160, 20, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_THRESH, 20, 210, 200, 30, 0, 0x0, 0 },
  
  { BUTTON_CreateIndirect, "Manual", ID_BUTTON_MANUAL, 20, 260, 90, 40, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Automatic", ID_BUTTON_AUTO, 130, 260, 90, 40, 0, 0x0, 0 }
};

static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int NCode;
  int Id;
  
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG: 
    WINDOW_SetBkColor(pMsg->hWin, GUI_WHITE);
    
    // LEFT Thermometer Setup
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_TC74);
   PROGBAR_SetSkinClassic(hItem);// Force classic skin so it allows red color!
    PROGBAR_SetMinMax(hItem, 0, 50); 
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); 
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); 
    
    // RIGHT Thermometer Setup
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_INT);
    PROGBAR_SetSkinClassic(hItem); // Force classic skin here too
    PROGBAR_SetMinMax(hItem, 0, 50); 
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
    // --- SLIDER LOGIC ADDED HERE ---
    case ID_SLIDER_THRESH:
      if (NCode == WM_NOTIFICATION_VALUE_CHANGED) {
         // Update the variable when slider moves
         fan_threshold = SLIDER_GetValue(pMsg->hWinSrc);
      }
      break;

    // --- BUTTON LOGIC ---
    case ID_BUTTON_AUTO: 
      if (NCode == WM_NOTIFICATION_RELEASED) {
        auto_mode = 1; 
      }
      break;
      
    case ID_BUTTON_MANUAL: 
      if (NCode == WM_NOTIFICATION_RELEASED) {
        auto_mode = 0; 
        if (manual_fan_state == 0) manual_fan_state = 1; // Toggle Fan On
        else manual_fan_state = 0;                       // Toggle Fan Off
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