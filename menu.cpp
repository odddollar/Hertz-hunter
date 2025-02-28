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
void drawScanMenu(menuStruct *menu, int rssiValues[60], int numFrequenciesToScan, int minFreq, int interval, int calibratedRssi[2], SemaphoreHandle_t mutex) {
  // Keeps small area at top and bottom for text display
  const int barYMin = 14;
  const int barYMax = 57;

  // Clear screen
  u8g2.clearBuffer();

  // Draw bottom numbers
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 64, "5645");
  u8g2.drawStr(55, 64, "5795");
  u8g2.drawStr(109, 64, "5945");

  // Draw selected frequency
  u8g2.setFont(u8g2_font_7x13_tf);
  String currentFrequency = String(menu->menuIndex * interval + minFreq) + "MHz";
  u8g2.drawStr(0, 13, currentFrequency.c_str());

  // Calculate width of each bar in graph by expanding until best fit
  int barWidth = 1;
  while ((barWidth + 1) * numFrequenciesToScan <= 128) {
    barWidth++;
  }

  // Calculate side padding offset for graph
  int padding = (128 - (barWidth * numFrequenciesToScan)) / 2;

  // Iterate through rssi values
  for (int i = 0; i < numFrequenciesToScan; i++) {
    int rssiValue;

    // Take mutex to safely modify in this thread
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      rssiValue = rssiValues[i];

      xSemaphoreGive(mutex);
    }

    // Clamp value between calibrated min and max
    rssiValue = std::clamp(rssiValue, calibratedRssi[0], calibratedRssi[1]);

    // Calculate height of individual bar
    int barHeight = map(rssiValue, calibratedRssi[0], calibratedRssi[1], 0, barYMax - barYMin);

    // Draw box with x-offset
    if (i == menu->menuIndex) {
      // Highlight selection
      u8g2.drawBox(i * barWidth + padding, barYMin, barWidth, barYMax - barYMin);
      u8g2.setDrawColor(0);
      u8g2.drawBox(i * barWidth + padding, barYMax - barHeight, barWidth, barHeight);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawBox(i * barWidth + padding, barYMax - barHeight, barWidth, barHeight);
    }
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
