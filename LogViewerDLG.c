#include "DIALOG.h"

// Define the ID numbers for our widgets
#define ID_WINDOW_0      (GUI_ID_USER + 0x00)
#define ID_PROGBAR_0     (GUI_ID_USER + 0x01)
#define ID_BUTTON_AUTO   (GUI_ID_USER + 0x02)
#define ID_BUTTON_MANUAL (GUI_ID_USER + 0x03)
#define ID_TEXT_0        (GUI_ID_USER + 0x04)

// This array defines exactly where every widget is placed on the screen
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  // { Widget Function, "Text", ID, X-pos, Y-pos, Width, Height, Flags, Extra, 0 }
  { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 240, 320, 0, 0x0, 0 },
  
  // The Mercury Column (Progress Bar). It is taller than it is wide, so it fills vertically.
  { PROGBAR_CreateIndirect, "Progbar", ID_PROGBAR_0, 100, 50, 40, 150, 0, 0x0, 0 }, 
  
  // The two buttons required by the assignment
  { BUTTON_CreateIndirect, "Auto", ID_BUTTON_AUTO, 20, 240, 80, 40, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Manual", ID_BUTTON_MANUAL, 140, 240, 80, 40, 0, 0x0, 0 },
  
  // A label to show the exact number above the column
  { TEXT_CreateIndirect, "Temp", ID_TEXT_0, 100, 20, 80, 20, 0, 0x0, 0 }
};

// This function handles the setup and coloring of the widgets
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    // 1. Set the background color to white
    WINDOW_SetBkColor(pMsg->hWin, GUI_WHITE);
    
    // 2. Setup the Mercury Column (Progress Bar)
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_0);
    PROGBAR_SetMinMax(hItem, 0, 50); // Scale from 0 to 50 degrees
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); // The filled "mercury" part is red
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); // The empty part is gray
    
    // 3. Setup the text label
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetFont(hItem, GUI_FONT_16_1);
    TEXT_SetText(hItem, "Temp: --");
    break;
    
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

// This is the function GUI_SingleThread.c calls to build the screen!
WM_HWIN CreateLogViewer(void);
WM_HWIN CreateLogViewer(void) {
  WM_HWIN hWin;
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}