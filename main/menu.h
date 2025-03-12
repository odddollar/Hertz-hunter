#ifndef MENU_H
#define MENU_H

#include <U8g2lib.h>

// Version information
#define VERSION "v1.0.0"
#define AUTHOR "By Simon Eason"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// Keeps small area at top and bottom for text display
#define BAR_Y_MIN 14
#define BAR_Y_MAX 57

// External declaration for display
// If using an OLED with an SH1106 chip then leave this be
// If using an OLED with an SSD1306 chip then comment out the SH1106 line and uncomment the SSD1306 line
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
// extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// Single menu item with icon
struct menuItemStruct {
  char *name;
  const unsigned char *icon;
};

// Whole menu with multiple items
struct menuStruct {
  char *name;
  menuItemStruct *menuItems;
  int menuItemsLength;
  int menuIndex;
};

// External declarations for menu items and menus
extern menuItemStruct mainMenuItems[];
extern menuItemStruct settingsMenuItems[];
extern menuItemStruct calibrationMenuItems[];
extern menuItemStruct scanIntervalMenuItems[];
extern menuItemStruct buzzerMenuItems[];
extern menuItemStruct batteryAlarmMenuItems[];
extern menuStruct menus[];

// Declarations for drawing menu functions
void updateMenuIcons(menuStruct *menu, int selected);
void drawSelectionMenu(menuStruct *menu, int batteryVoltage);
void drawScanMenu(menuStruct *menu, int rssiValues[61], int numFrequenciesToScan, int minFreq, int interval, int calibratedRssi[2], SemaphoreHandle_t mutex);
void drawAboutMenu(menuStruct *menu);

#endif
