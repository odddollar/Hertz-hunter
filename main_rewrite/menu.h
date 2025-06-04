#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "about.h"
#include "api.h"
#include "battery.h"
#include "bitmaps.h"
#include "buzzer.h"
#include "RX5808.h"
#include "settings.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#define DEBOUNCE_DELAY 150

// How long button has to be held to be long-pressed
#define LONG_PRESS_DURATION (500 - DEBOUNCE_DELAY)

// Keeps small area at top and bottom for text display on scan menu
#define BAR_Y_MIN 14
#define BAR_Y_MAX 57

// Enum for different menus
// Order is important from initMenus()
enum MenuIndex {
  MAIN,
  SCAN,
  SETTINGS,
  ABOUT,
  ADVANCED,
  SCAN_INTERVAL,
  BUZZER,
  BATTERY_ALARM,
  WIFI,
  CALIBRATION,
  MENU_COUNT  // For array bounds checking
};

// Holds menu state, and navigation and drawing functions
class Menu {
public:
  Menu(uint8_t p_p, uint8_t s_p, uint8_t n_p, Settings *s, Buzzer *b, RX5808 *r, Api *a);
  void begin();
  void handleButtons();
  void clearBuffer();
  void sendBuffer();
  void drawMenu();
  void drawBatteryVoltage(int voltage);

private:
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

  void drawSelectionMenu();
  void drawScanMenu();
  void drawAboutMenu();
  void drawWifiMenu();
  void updateSettingsOptionIcons(menuStruct *menu, int selectedIndex);
  void initMenus();
  int xTextCentre(const char *text, int fontCharWidth);

  menuItemStruct mainMenuItems[3];
  menuItemStruct settingsMenuItems[3];
  menuItemStruct scanIntervalMenuItems[3];
  menuItemStruct buzzerMenuItems[2];
  menuItemStruct batteryAlarmMenuItems[3];
  menuItemStruct advancedMenuItems[2];
  menuItemStruct calibrationMenuItems[2];
  menuStruct menus[MENU_COUNT];

  MenuIndex menuIndex;

  uint8_t previous_pin;
  uint8_t select_pin;
  uint8_t next_pin;

  // Used to handle long-pressing SELECT to go back
  unsigned long selectButtonPressTime;
  bool selectButtonHeld;

  Settings *settings;
  Buzzer *buzzer;
  RX5808 *module;
  Api *api;

  // If using an OLED with an SH1106 chip then leave this be
  // If using an OLED with an SSD1306 chip then comment out the SH1106 line and uncomment the SSD1306 line
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
  // U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
};

#endif
