#ifndef MENU_H
#define MENU_H

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

#endif
