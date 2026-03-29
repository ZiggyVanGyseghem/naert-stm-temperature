

#include "DIALOG.h"

// --- GUI Widget Unique IDs ---
// Every element on the screen needs a unique ID so the code knows what we are touching.
#define ID_WINDOW_0        (GUI_ID_USER + 0x00) // The main background window
#define ID_PROGBAR_TC74    (GUI_ID_USER + 0x01) // Left mercury column
#define ID_PROGBAR_INT     (GUI_ID_USER + 0x02) // Right mercury column
#define ID_TEXT_TC74       (GUI_ID_USER + 0x03) // Left temperature text
#define ID_TEXT_INT        (GUI_ID_USER + 0x04) // Right temperature text
#define ID_SLIDER_THRESH   (GUI_ID_USER + 0x05) // Slider for fan threshold
#define ID_TEXT_THRESH     (GUI_ID_USER + 0x06) // Text showing slider value
#define ID_BUTTON_MANUAL   (GUI_ID_USER + 0x07) // Manual mode button
#define ID_BUTTON_AUTO     (GUI_ID_USER + 0x08) // Auto mode button
#define ID_BUTTON_SENSOR   (GUI_ID_USER + 0x09) // Sensor toggle button

// --- External Global Variables ---
// These "extern" keywords tell the compiler: "These variables exist in 
// GUI_SingleThread.c. Do not create new ones, just link to those!"
extern int auto_mode;         // 1 = Auto, 0 = Manual
extern int manual_fan_state;  // 1 = Fan ON, 0 = Fan OFF
extern int fan_threshold;     // Threshold set by the slider
extern int right_bar_mode;    // 0 = Internal, 1 = Gyroscope

// =========================================================================
// UI BLUEPRINT (The layout of the screen)
// =========================================================================
// This array defines the Type, Text, ID, X-pos, Y-pos, Width, and Height of every widget.
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Main Screen", ID_WINDOW_0, 0, 0, 240, 320, 0, 0x0, 0 },
  
  // Left Side (Always dedicated to the external TC74 sensor)
  { TEXT_CreateIndirect, "22C", ID_TEXT_TC74, 30, 40, 60, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "ProgbarTC74", ID_PROGBAR_TC74, 40, 65, 40, 110, PROGBAR_CF_VERTICAL, 0x0, 0 }, 
  
  // Right Side (Switches between Internal Processor Diode or Gyroscope)
  { TEXT_CreateIndirect, "20C", ID_TEXT_INT, 150, 40, 60, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "ProgbarInt", ID_PROGBAR_INT, 160, 65, 40, 110, PROGBAR_CF_VERTICAL, 0x0, 0 }, 

  // Sensor Toggle Button (Placed neatly under the right bar)
  { BUTTON_CreateIndirect, "View: INT", ID_BUTTON_SENSOR, 140, 185, 80, 25, 0, 0x0, 0 },

  // Bottom Controls (Fan Threshold Slider)
  { TEXT_CreateIndirect, "Start Fan at: 25 C", ID_TEXT_THRESH, 40, 220, 160, 20, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_THRESH, 20, 240, 200, 30, 0, 0x0, 0 },
  
  // Bottom Controls (System State Buttons)
  { BUTTON_CreateIndirect, "Manual", ID_BUTTON_MANUAL, 20, 280, 90, 35, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Automatic", ID_BUTTON_AUTO, 130, 280, 90, 35, 0, 0x0, 0 }
};

// =========================================================================
// UI CALLBACK FUNCTION (The Event Handler)
// =========================================================================
// This function acts like a switchboard. Every time the screen is drawn or 
// touched, the emWin OS sends a message (pMsg) here to be processed.
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem; // Handle to temporarily hold a widget
  int NCode;     // Notification code (e.g., "Clicked", "Released", "Value Changed")
  int Id;        // ID of the widget that triggered the event
  
  switch (pMsg->MsgId) {
      
  // --- PHASE 1: INITIALIZATION ---
  // Triggered once when the screen is first created to set up visual limits and colors.
  case WM_INIT_DIALOG: 
    WINDOW_SetBkColor(pMsg->hWin, GUI_WHITE); // Set background to white
    
    // Configure Left Progress Bar (TC74)
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_TC74);
    PROGBAR_SetSkinClassic(hItem); 
    PROGBAR_SetMinMax(hItem, 0, 50); // Room temps usually stay between 0-50C
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); 
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); 
    
    // Configure Right Progress Bar (Internal/Gyro)
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_INT);
    PROGBAR_SetSkinClassic(hItem); 
    PROGBAR_SetMinMax(hItem, 0, 100); // Expanded to 100C because processors get hot!
    PROGBAR_SetBarColor(hItem, 0, GUI_RED); 
    PROGBAR_SetBarColor(hItem, 1, GUI_LIGHTGRAY); 
    
    // Configure the Fan Threshold Slider
    hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_THRESH);
    SLIDER_SetRange(hItem, 0, 50); // Slider limits (0C to 50C)
    SLIDER_SetValue(hItem, 25);    // Default starting position
    
    // Bold the text elements and center align them
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

  // --- PHASE 2: EVENT HANDLING ---
  // Triggered whenever the user interacts with the UI (touching, dragging)
  case WM_NOTIFY_PARENT: 
    Id    = WM_GetId(pMsg->hWinSrc); // Which widget was touched?
    NCode = pMsg->Data.v;            // What kind of touch was it?
    
    switch(Id) {
        
    // 1. If the user drags the SLIDER
    case ID_SLIDER_THRESH:
      if (NCode == WM_NOTIFICATION_VALUE_CHANGED) {
         // Update the global fan_threshold variable immediately
         fan_threshold = SLIDER_GetValue(pMsg->hWinSrc);
      }
      break;

    // 2. If the user taps the SENSOR TOGGLE button
    case ID_BUTTON_SENSOR:
      if (NCode == WM_NOTIFICATION_RELEASED) { // Only trigger when finger is lifted
        if (right_bar_mode == 0) {
            // Switch from Internal -> Gyro
            right_bar_mode = 1; 
            BUTTON_SetText(pMsg->hWinSrc, "View: GYRO");
        } else {
            // Switch from Gyro -> Internal
            right_bar_mode = 0; 
            BUTTON_SetText(pMsg->hWinSrc, "View: INT");
        }
      }
      break;

    // 3. If the user taps the AUTOMATIC button
    case ID_BUTTON_AUTO: 
      if (NCode == WM_NOTIFICATION_RELEASED) {
          auto_mode = 1; // Tell the backend thread to use threshold logic
      }
      break;
      
    // 4. If the user taps the MANUAL button
    case ID_BUTTON_MANUAL: 
      if (NCode == WM_NOTIFICATION_RELEASED) {
        auto_mode = 0; // Tell the backend thread to ignore temperatures
        
        // Toggle the manual fan state on/off with every tap
        if (manual_fan_state == 0) manual_fan_state = 1; 
        else manual_fan_state = 0;                       
      }
      break;
    }
    break;
    
  // Let the emWin system handle any messages we don't care about
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

// =========================================================================
// PUBLIC CREATION FUNCTION
// =========================================================================
// This function is called by GUI_SingleThread.c to actually spawn the window.
WM_HWIN CreateLogViewer(void);
WM_HWIN CreateLogViewer(void) {
  // Creates a non-blocking dialog box using our Blueprint and our Callback handler
  return GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}