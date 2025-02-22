#include "menu.h"
#include "bitmaps.h"

// Create items in main menu
menuItemStruct mainMenuItems[] = {
  { "Scan", bitmap_Scan },
  { "Settings", bitmap_Settings },
  { "About", bitmap_About }
};

// Create items in settings menu
menuItemStruct settingsMenuItems[] = {
  { "Scan interval", bitmap_Interval },
  { "Buzzer", bitmap_Buzzer },
  { "Bat. alarm", bitmap_Alarm }
};

// Create items in calibration menu
menuItemStruct calibrationMenuItems[] = {
  { "Calib. high", bitmap_Scan },
  { "Calib. low", bitmap_ScanLow }
};

// Create items in scan interval menu
menuItemStruct scanIntervalMenuItems[] = {
  { "5MHz", bitmap_Selected },
  { "10MHz", bitmap_Blank },
  { "20MHz", bitmap_Blank }
};

// Create items in buzzer menu
menuItemStruct buzzerMenuItems[] = {
  { "On", bitmap_Selected },
  { "Off", bitmap_Blank },
};

// Create items in battery alarm menu
menuItemStruct batteryAlarmMenuItems[] = {
  { "3.6v", bitmap_Selected },
  { "3.3v", bitmap_Blank },
  { "3.0v", bitmap_Blank }
};

// Struct containing all menus
menuStruct menus[] = {
  { "Hertz Hunter", mainMenuItems, 3, 0 },
  { "Scan", nullptr, 60, 0 },  // 60 frequencies to scan. TODO: Make dynamic with scan interval setting
  { "Settings", settingsMenuItems, 3, 0 },
  { "About", nullptr, 1, 0 },  // Given length of 1 to prevent zero-division
  { "Calibration", calibrationMenuItems, 2, 0 },
  { "Scan interval", scanIntervalMenuItems, 3, 0 },
  { "Buzzer", buzzerMenuItems, 2, 0 },
  { "Bat. alarm", batteryAlarmMenuItems, 3, 0 }
};
