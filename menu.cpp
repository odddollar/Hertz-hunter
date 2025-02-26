#include "menu.h"
#include "bitmaps.h"
#include "version.h"

// Setup display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

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
  { "Scan", nullptr, 60, 0 },
  { "Settings", settingsMenuItems, 3, 0 },
  { "About", nullptr, 1, 0 },  // Given length of 1 to prevent zero-division
  { "Calibration", calibrationMenuItems, 2, 0 },
  { "Scan interval", scanIntervalMenuItems, 3, 0 },
  { "Buzzer", buzzerMenuItems, 2, 0 },
  { "Bat. alarm", batteryAlarmMenuItems, 3, 0 }
};

// Update menu icons based on settings
void updateMenuIcons(menuStruct *menu, int selected) {
  for (int i = 0; i < menu->menuItemsLength; i++) {
    if (i == selected) {
      menu->menuItems[i].icon = bitmap_Selected;
    } else {
      menu->menuItems[i].icon = bitmap_Blank;
    }
  }
}

// Draw selection menu with content provided
void drawSelectionMenu(menuStruct *menu) {
  // Clear screen
  u8g2.clearBuffer();

  // Calculate x position of title
  // 128 is width of display, 8 is width of font char
  int xPos = (128 - (strlen(menu->name) * 8)) / 2;

  // Draw title
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(xPos, 13, menu->name);
  u8g2.setFont(u8g2_font_7x13_tf);

  // Draw menu items
  for (int i = 0; i < menu->menuItemsLength; i++) {
    if (i == menu->menuIndex) {
      // Highlight selection
      u8g2.drawBox(0, 16 + (i * 16), 128, 16);
      u8g2.setDrawColor(0);
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menu->menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menu->menuItems[i].name);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menu->menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menu->menuItems[i].name);
    }
  }

  // Send drawing to display
  u8g2.sendBuffer();
}

// Draw graph of scanned rssi values
void drawScanMenu(int rssiValues[60], int numFrequenciesToScan) {
  // Clear screen
  u8g2.clearBuffer();

  // Calculate width of each bar in graph
  // Use 120 pixels of 128 width
  int barWidth = 120 / numFrequenciesToScan;

  // Iterate through rssi values
  for (int i = 0; i < numFrequenciesToScan; i++) {
    // Calculate height of individual bar
    int barHeight = map(rssiValues[i], 512, 2048, 0, 64);

    u8g2.drawBox(i * barWidth + 4, 64 - barHeight, barWidth, barHeight);
  }

  // Send drawing to display
  u8g2.sendBuffer();
}

// Draw static content on about menu
void drawAboutMenu() {
  // Clear screen
  u8g2.clearBuffer();

  // Draw title
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(46, 13, "About");
  u8g2.setFont(u8g2_font_7x13_tf);

  // Draw summary text
  u8g2.drawStr(15, 28, "5.8GHz scanner");

  // Draw version
  u8g2.drawStr(43, 44, version);

  // Draw author
  u8g2.drawStr(15, 60, author);

  // Send drawing to display
  u8g2.sendBuffer();
}
