#ifndef MENU_H
#define MENU_H

#include <U8g2lib.h>

// External declaration for display
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

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
void drawSelectionMenu(menuStruct *menu);
void drawScanMenu(int rssiValues[60], int numFrequenciesToScan);
void drawAboutMenu();

#endif
