#ifndef MENU_H
#define MENU_H

#include <U8g2lib.h>
#include "bitmaps.h"
#include "buzzer.h"
#include "RX5808.h"
#include "settings.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#define DEBOUNCE_DELAY 150

// Holds menu state, and navigation and drawing functions
class Menu {
public:
  Menu(uint8_t p_p, uint8_t s_p, uint8_t n_p, Settings *s, Buzzer *b);
  void begin();
  void handleButtons();
  void drawMenu();
  void drawBatteryVoltage(int voltage);

private:
  void initMenus();

  // Menu data structures
  struct menuItemStruct {
    const char *name;
    const unsigned char *icon;
  };

  struct menuStruct {
    const char *title;
    menuItemStruct *menuItems;
    int menuItemsLength;
    int menuIndex;
  };

  menuItemStruct mainMenuItems[3];
  menuItemStruct settingsMenuItems[3];
  menuItemStruct scanIntervalMenuItems[3];
  menuItemStruct buzzerMenuItems[2];
  menuItemStruct batteryAlarmMenuItems[3];
  menuItemStruct advancedMenuItems[2];
  menuItemStruct calibrationMenuItems[2];
  menuStruct menus[10];

  int menuIndex;
  uint8_t previous_pin;
  uint8_t select_pin;
  uint8_t next_pin;
  Settings *settings;
  Buzzer *buzzer;

  // If using an OLED with an SH1106 chip then leave this be
  // If using an OLED with an SSD1306 chip then comment out the SH1106 line and uncomment the SSD1306 line
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
  // U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
};

#endif
