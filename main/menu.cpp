#include "menu.h"
#include "bitmaps.h"

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
  // 8 is width of font char
  int xPos = (DISPLAY_WIDTH - (strlen(menu->name) * 8)) / 2;

  // Draw title
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(xPos, 13, menu->name);
  u8g2.setFont(u8g2_font_7x13_tf);

  // Draw menu items
  for (int i = 0; i < menu->menuItemsLength; i++) {
    if (i == menu->menuIndex) {
      // Highlight selection
      u8g2.drawBox(0, 16 + (i * 16), DISPLAY_WIDTH, 16);
      u8g2.setDrawColor(0);
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menu->menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menu->menuItems[i].name);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menu->menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menu->menuItems[i].name);
    }
  }

  // Draw extra text for calibration menu
  if (strcmp(menu->name, "Calibration") == 0) {
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(17, 60, "Set to 5806MHz (R5)");
  }

  // Send drawing to display
  u8g2.sendBuffer();
}

// Draw graph of scanned rssi values
void drawScanMenu(menuStruct *menu, int rssiValues[61], int numFrequenciesToScan, int minFreq, int interval, int calibratedRssi[2], SemaphoreHandle_t mutex) {
  // Calculate width of each bar in graph by expanding until best fit
  int barWidth = 1;
  while ((barWidth + 1) * numFrequenciesToScan <= DISPLAY_WIDTH) {
    barWidth++;
  }

  // Calculate side padding offset for graph
  int padding = (DISPLAY_WIDTH - (barWidth * numFrequenciesToScan)) / 2;

  // Clear screen
  u8g2.clearBuffer();

  // Draw bottom numbers
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, DISPLAY_HEIGHT, "5645");
  u8g2.drawStr(55, DISPLAY_HEIGHT, "5795");
  u8g2.drawStr(109, DISPLAY_HEIGHT, "5945");

  // Draw selected frequency
  u8g2.setFont(u8g2_font_7x13_tf);
  char currentFrequency[8];
  snprintf(currentFrequency, sizeof(currentFrequency), "%dMHz", menu->menuIndex * interval + minFreq);
  u8g2.drawStr(0, 13, currentFrequency);

  // Copy rssi values safely
  int currentFrequencyRssi;
  int rssiValuesCopy[61];
  if (xSemaphoreTake(mutex, portMAX_DELAY)) {
    currentFrequencyRssi = rssiValues[menu->menuIndex];
    memcpy(rssiValuesCopy, rssiValues, sizeof(int) * numFrequenciesToScan);
    xSemaphoreGive(mutex);
  }

  // Clamp and convert rssi to percentage
  currentFrequencyRssi = std::clamp(currentFrequencyRssi, calibratedRssi[0], calibratedRssi[1]);
  char percentageStr[5];
  snprintf(percentageStr, sizeof(percentageStr), "%d%%", map(currentFrequencyRssi, calibratedRssi[0], calibratedRssi[1], 0, 100));

  // Draw rssi percentage accounting for changes from 3 to 4 characters
  int percentageX = DISPLAY_WIDTH - (strlen(percentageStr) * 7) + 1;
  u8g2.drawStr(percentageX, 13, percentageStr);

  // Iterate through rssi values
  for (int i = 0; i < numFrequenciesToScan; i++) {
    // Clamp value between calibrated min and max
    int rssiValue = std::clamp(rssiValuesCopy[i], calibratedRssi[0], calibratedRssi[1]);

    // Calculate height of individual bar
    int barHeight = map(rssiValue, calibratedRssi[0], calibratedRssi[1], 0, BAR_Y_MAX - BAR_Y_MIN);

    // Draw box with x-offset
    if (i == menu->menuIndex) {
      // Highlight selection
      u8g2.drawBox(i * barWidth + padding, BAR_Y_MIN, barWidth, BAR_Y_MAX - BAR_Y_MIN);
      u8g2.setDrawColor(0);
      u8g2.drawBox(i * barWidth + padding, BAR_Y_MAX - barHeight, barWidth, barHeight);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawBox(i * barWidth + padding, BAR_Y_MAX - barHeight, barWidth, barHeight);
    }
  }

  // Send drawing to display
  u8g2.sendBuffer();
}

// Draw static content on about menu
void drawAboutMenu(menuStruct *menu) {
  // Clear screen
  u8g2.clearBuffer();

  // Calculate x position of title
  // 8 is width of font char
  int xPos = (DISPLAY_WIDTH - (strlen(menu->name) * 8)) / 2;

  // Draw title
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(46, 13, menu->name);
  u8g2.setFont(u8g2_font_7x13_tf);

  // Draw summary text
  u8g2.drawStr(15, 28, "5.8GHz scanner");

  // Draw version
  u8g2.drawStr(43, 44, VERSION);

  // Draw author
  u8g2.drawStr(15, 60, AUTHOR);

  // Send drawing to display
  u8g2.sendBuffer();
}
