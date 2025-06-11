#include "menu.h"

Menu::Menu(uint8_t p_p, uint8_t s_p, uint8_t n_p, Settings *s, Buzzer *b, RX5808 *r, Api *a)
  : menuIndex(MAIN),
    previous_pin(p_p), select_pin(s_p), next_pin(n_p),
    selectButtonPressTime(0), selectButtonHeld(false),
    settings(s), buzzer(b), module(r), api(a),
    u8g2(U8G2_R0, U8X8_PIN_NONE) {
}

// Begin menu object
void Menu::begin() {
  initMenus();

  // Can't call in constructor as pulldown overwritten during boot before setup() called
  pinMode(previous_pin, INPUT_PULLDOWN);
  pinMode(select_pin, INPUT_PULLDOWN);
  pinMode(next_pin, INPUT_PULLDOWN);

  u8g2.begin();
  u8g2.clearBuffer();
}

// Handle navigation between menus
// Manipulates the internal menuIndex variable
void Menu::handleButtons() {
  // Check menu button presses
  int prevPressed = digitalRead(previous_pin);
  int selectPressed = digitalRead(select_pin);
  int nextPressed = digitalRead(next_pin);

  // Hidden reset function
  if (prevPressed == HIGH && selectPressed == HIGH && nextPressed == HIGH) {
    settings->clearReset();
  }

  // Update length of scan menu
  menus[SCAN].menuItemsLength = (SCAN_FREQUENCY_RANGE / settings->scanInterval.get()) + 1;  // +1 for final number inclusion

  // Move between menu items
  if (nextPressed == HIGH || prevPressed == HIGH) {
    int direction = (nextPressed == HIGH) ? 1 : -1;
    menus[menuIndex].menuIndex = (menus[menuIndex].menuIndex + direction + menus[menuIndex].menuItemsLength) % menus[menuIndex].menuItemsLength;

    // Sound buzzer on button press if necessary
    if (settings->buzzer.get()) buzzer->buzz();

    // Delay for button debouncing
    delay(DEBOUNCE_DELAY);
  }

  // Handle pressing and holding SELECT to go back
  if (selectPressed == HIGH) {
    if (selectButtonPressTime == 0) {  // Button just pressed so record time
      selectButtonPressTime = millis();

      // Sound buzzer on button press if necessary
      if (settings->buzzer.get()) buzzer->buzz();
    } else if (!selectButtonHeld && millis() - selectButtonPressTime > LONG_PRESS_DURATION) {  // Held longer than threshold register long press
      switch (menuIndex) {
        case MAIN: menuIndex = ADVANCED; break;                             // If on main menu, go to advanced
        case SCAN_INTERVAL ... BATTERY_ALARM: menuIndex = SETTINGS; break;  // If on individual settings menu, go to settings
        case WIFI ... CALIBRATION: menuIndex = ADVANCED; break;             // If on individual advanced menu, go to advanced
        default: menuIndex = MAIN; break;                                   // Otherwise, go back to main menu
      }

      selectButtonHeld = true;

      // Sound double buzz on back if necessary
      if (settings->buzzer.get()) buzzer->doubleBuzz();
    }

    // Delay for button debouncing
    delay(DEBOUNCE_DELAY);

    // Immediately end and wait for next iteration of loop()
    return;
  }

  // If SELECT button was pressed but not held, use as SELECT rather than BACK
  if (selectButtonPressTime > 0 && !selectButtonHeld) {
    switch (menuIndex) {
      case MAIN:  // Handle SELECT on main menu
        switch (menus[MAIN].menuIndex) {
          case 0: menuIndex = SCAN; break;      // Go to scan menu
          case 1: menuIndex = SETTINGS; break;  // Go to settings menu
          case 2: menuIndex = ABOUT; break;     // Go to about menu
        }
        break;
      case SCAN:  // Handle SELECT on scan menu
        module->lowband.set(!module->lowband.get());
        break;
      case SETTINGS:  // Handle SELECT on settings menu
        switch (menus[SETTINGS].menuIndex) {
          case 0: menuIndex = SCAN_INTERVAL; break;  // Go to scan interval menu
          case 1: menuIndex = BUZZER; break;         // Go to buzzer menu
          case 2: menuIndex = BATTERY_ALARM; break;  // Go to battery alarm menu
        }
        break;
      case ADVANCED:  // Handle SELECT on advanced menu
        switch (menus[ADVANCED].menuIndex) {
          case 0: menuIndex = WIFI; break;         // Go to Wi-Fi menu
          case 1: menuIndex = CALIBRATION; break;  // Go to calibration menu
        }
        break;
      case SCAN_INTERVAL ... BATTERY_ALARM:  // Handle SELECT on individual settings options
        switch (menuIndex) {
          case SCAN_INTERVAL:  // Update scan interval settings and icons
            settings->scanIntervalIndex.set(menus[menuIndex].menuIndex);
            menus[SCAN].menuIndex = 0;
            break;
          case BUZZER:  // Update buzzer settings and icons
            settings->buzzerIndex.set(menus[menuIndex].menuIndex);
            break;
          case BATTERY_ALARM:  // Update battery alarm settings and icons
            settings->batteryAlarmIndex.set(menus[menuIndex].menuIndex);
            break;
        }
        break;
      case CALIBRATION:  // Handle SELECT on calibration menu
        switch (menus[CALIBRATION].menuIndex) {
          case 0: module->calibrate(true); break;   // Calibrate high rssi
          case 1: module->calibrate(false); break;  // Calibrate low rssi
        }
        break;
    }
  }

  // Reset SELECT when button released
  selectButtonPressTime = 0;
  selectButtonHeld = false;
}

// Clear display buffer
void Menu::clearBuffer() {
  u8g2.clearBuffer();
}

// Send data to display buffer
void Menu::sendBuffer() {
  u8g2.sendBuffer();
}

// Draw current menu
void Menu::drawMenu() {
  // Draw title, but not for scan menu
  if (menuIndex != SCAN) {
    u8g2.setFont(u8g2_font_8x13B_tf);
    const char *title = menus[menuIndex].title;
    u8g2.drawStr(xTextCentre(title, 8), 13, title);
    u8g2.setFont(u8g2_font_7x13_tf);
  }

  // Update in-memory icons for individual settings options
  if (menuIndex >= SCAN_INTERVAL && menuIndex <= BATTERY_ALARM) {
    updateSettingsOptionIcons(&menus[SCAN_INTERVAL], settings->scanIntervalIndex.get());
    updateSettingsOptionIcons(&menus[BUZZER], settings->buzzerIndex.get());
    updateSettingsOptionIcons(&menus[BATTERY_ALARM], settings->batteryAlarmIndex.get());
  }

  // Call appropriate draw function
  switch (menuIndex) {
    case SCAN:  // Draw scan menu
      module->startScan();
      drawScanMenu();
      break;
    case ABOUT:  // Draw about menu
      drawAboutMenu();
      break;
    case WIFI:  // Draw Wi-Fi menu
      module->startScan();
      api->startWifi();
      drawWifiMenu();
      break;
    default:  // Draw selection menu with options
      module->stopScan();
      api->stopWifi();
      drawSelectionMenu();
      break;
  }
}

// Display battery voltage in bottom corner of main menu
void Menu::drawBatteryVoltage(int voltage) {
  // Draw voltage only display if on main menu
  if (menuIndex == MAIN) {
    // Format voltage reading
    char formattedVoltage[5];
    snprintf(formattedVoltage, sizeof(formattedVoltage), "%d.%dv", voltage / 10, voltage % 10);

    // Set font colour to inverted if selected bottom item
    u8g2.setDrawColor(menus[MAIN].menuIndex == 2 ? 0 : 1);
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(109, DISPLAY_HEIGHT, formattedVoltage);
    u8g2.setDrawColor(1);
  }
}

// Generic function for drawing menus with multiple options
void Menu::drawSelectionMenu() {
  // Draw menu items
  for (int i = 0; i < menus[menuIndex].menuItemsLength; i++) {
    if (i == menus[menuIndex].menuIndex) {
      // Highlight selection
      u8g2.drawBox(0, 16 + (i * 16), DISPLAY_WIDTH, 16);
      u8g2.setDrawColor(0);
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menus[menuIndex].menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menus[menuIndex].menuItems[i].name);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menus[menuIndex].menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menus[menuIndex].menuItems[i].name);
    }
  }

  // Draw extra text for calibration menu
  if (menuIndex == CALIBRATION) {
    u8g2.setFont(u8g2_font_5x7_tf);
    const char *text = "Set to 5800MHz (F4)";
    u8g2.drawStr(xTextCentre(text, 5), 60, text);
  }
}

// Draw graph of scanned rssi values
void Menu::drawScanMenu() {
  // Calculate number of scanned values based off of interval
  int interval = settings->scanInterval.get();
  int numScannedValues = (SCAN_FREQUENCY_RANGE / interval) + 1;  // +1 for final number inclusion

  // Calculate width of each bar in graph by expanding until best fit
  int barWidth = 1;
  while ((barWidth + 1) * numScannedValues <= DISPLAY_WIDTH) {
    barWidth++;
  }

  // Calculate side padding offset for graph
  int padding = (DISPLAY_WIDTH - (barWidth * numScannedValues)) / 2;

  // Get min and max calibrated rssi
  int minRssi = settings->lowCalibratedRssi.get();
  int maxRssi = settings->highCalibratedRssi.get();

  // Draw bottom numbers
  u8g2.setFont(u8g2_font_5x7_tf);
  if (module->lowband.get()) {
    u8g2.drawStr(0, DISPLAY_HEIGHT, "5345");
    u8g2.drawStr(55, DISPLAY_HEIGHT, "5495");
    u8g2.drawStr(109, DISPLAY_HEIGHT, "5645");
  } else {
    u8g2.drawStr(0, DISPLAY_HEIGHT, "5645");
    u8g2.drawStr(55, DISPLAY_HEIGHT, "5795");
    u8g2.drawStr(109, DISPLAY_HEIGHT, "5945");
  }

  // Draw high or low band
  u8g2.setFont(u8g2_font_7x13_tf);
  if (module->lowband.get()) {
    u8g2.drawStr(0, 13, "LOW");
  } else {
    u8g2.drawStr(0, 13, "HIGH");
  }

  // Draw selected frequency
  char currentFrequency[8];
  int min_freq = module->lowband.get() ? LOWBAND_MIN_FREQUENCY : HIGHBAND_MIN_FREQUENCY;
  snprintf(currentFrequency, sizeof(currentFrequency), "%dMHz", menus[SCAN].menuIndex * interval + min_freq);
  u8g2.drawStr(xTextCentre(currentFrequency, 7), 13, currentFrequency);

  // Safely get current rssi
  int currentFrequencyRssi;
  if (xSemaphoreTake(module->scanMutex, portMAX_DELAY)) {
    currentFrequencyRssi = module->rssiValues.get(menus[SCAN].menuIndex);

    xSemaphoreGive(module->scanMutex);
  }

  // Clamp and convert rssi to percentage
  currentFrequencyRssi = std::clamp(currentFrequencyRssi, minRssi, maxRssi);
  char percentageStr[5];
  snprintf(percentageStr, sizeof(percentageStr), "%d%%", map(currentFrequencyRssi, minRssi, maxRssi, 0, 100));

  // Draw rssi percentage accounting for changes from 3 to 4 characters
  int percentageX = DISPLAY_WIDTH - (strlen(percentageStr) * 7) + 1;
  u8g2.drawStr(percentageX, 13, percentageStr);

  // Iterate through rssi values
  for (int i = 0; i < numScannedValues; i++) {
    // Safely get current rssi
    int rssi;
    if (xSemaphoreTake(module->scanMutex, portMAX_DELAY)) {
      rssi = module->rssiValues.get(i);

      xSemaphoreGive(module->scanMutex);
    }

    // Clamp rssi between calibrated values
    rssi = std::clamp(rssi, minRssi, maxRssi);

    // Calculate height of individual bar
    int barHeight = map(rssi, minRssi, maxRssi, 0, BAR_Y_MAX - BAR_Y_MIN);

    // Draw box with x-offset
    // Highlight selection
    if (i == menus[SCAN].menuIndex) {
      u8g2.drawBox(i * barWidth + padding, BAR_Y_MIN, barWidth, BAR_Y_MAX - BAR_Y_MIN);
      u8g2.setDrawColor(0);
      u8g2.drawBox(i * barWidth + padding, BAR_Y_MAX - barHeight, barWidth, barHeight);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawBox(i * barWidth + padding, BAR_Y_MAX - barHeight, barWidth, barHeight);
    }
  }
}

// Draw static content on about menu
void Menu::drawAboutMenu() {
  const char *info = "5.8GHz scanner";
  u8g2.drawStr(xTextCentre(info, 7), 28, info);

  u8g2.drawStr(xTextCentre(VERSION, 7), 44, VERSION);

  u8g2.drawStr(xTextCentre(AUTHOR, 7), 60, AUTHOR);
}

// Draw static content on Wi-Fi menu
void Menu::drawWifiMenu() {
  // Draw SSID
  u8g2.setFont(u8g2_font_7x13B_tf);
  u8g2.drawStr(11, 28, "ID");
  u8g2.setFont(u8g2_font_7x13_tf);
  u8g2.drawStr(30, 28, WIFI_SSID);

  // Draw password
  u8g2.setFont(u8g2_font_7x13B_tf);
  u8g2.drawStr(4, 44, "PWD");
  u8g2.setFont(u8g2_font_7x13_tf);
  u8g2.drawStr(30, 44, WIFI_PASSWORD);

  // Draw IP
  u8g2.setFont(u8g2_font_7x13B_tf);
  u8g2.drawStr(11, 60, "IP");
  if (strlen(WIFI_IP) < 15) {  // If not 15 characters use regular font
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(30, 60, WIFI_IP);
  } else {  // If 15 characters use smaller font, otherwise last digit off screen
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(30, 59, WIFI_IP);
  }
}

// Update icons for selected settings options
void Menu::updateSettingsOptionIcons(menuStruct *menu, int selectedIndex) {
  for (int i = 0; i < menu->menuItemsLength; i++) {
    if (i == selectedIndex) {
      menu->menuItems[i].icon = bitmap_Selected;
    } else {
      menu->menuItems[i].icon = bitmap_Blank;
    }
  }
}

// Initialise menu structures
void Menu::initMenus() {
  // Main menu
  mainMenuItems[0] = { "Scan", bitmap_Scan };
  mainMenuItems[1] = { "Settings", bitmap_Settings };
  mainMenuItems[2] = { "About", bitmap_About };

  // Settings menu
  settingsMenuItems[0] = { "Scan interval", bitmap_Interval };
  settingsMenuItems[1] = { "Buzzer", bitmap_Buzzer };
  settingsMenuItems[2] = { "Bat. alarm", bitmap_Alarm };

  // Scan Interval menu
  scanIntervalMenuItems[0] = { "5MHz", bitmap_Blank };
  scanIntervalMenuItems[1] = { "10MHz", bitmap_Blank };
  scanIntervalMenuItems[2] = { "20MHz", bitmap_Blank };

  // Buzzer menu
  buzzerMenuItems[0] = { "On", bitmap_Blank };
  buzzerMenuItems[1] = { "Off", bitmap_Blank };

  // Battery Alarm menu
  batteryAlarmMenuItems[0] = { "3.6v", bitmap_Blank };
  batteryAlarmMenuItems[1] = { "3.3v", bitmap_Blank };
  batteryAlarmMenuItems[2] = { "3.0v", bitmap_Blank };

  // Advanced menu
  advancedMenuItems[0] = { "Wi-Fi", bitmap_Wifi };
  advancedMenuItems[1] = { "Calibration", bitmap_Calibration };

  // Calibration menu
  calibrationMenuItems[0] = { "Calib. high", bitmap_Wifi };
  calibrationMenuItems[1] = { "Calib. low", bitmap_WifiLow };

  // Menus
  menus[0] = { "Hertz Hunter", mainMenuItems, 3, 0 };
  menus[1] = { "Scan", nullptr, MAX_FREQUENCIES_SCANNED, 0 };
  menus[2] = { "Settings", settingsMenuItems, 3, 0 };
  menus[3] = { "About", nullptr, 1, 0 };
  menus[4] = { "Advanced", advancedMenuItems, 2, 0 };
  menus[5] = { "Scan interval", scanIntervalMenuItems, 3, 0 };
  menus[6] = { "Buzzer", buzzerMenuItems, 2, 0 };
  menus[7] = { "Bat. alarm", batteryAlarmMenuItems, 3, 0 };
  menus[8] = { "Wi-Fi", nullptr, 1, 0 };
  menus[9] = { "Calibration", calibrationMenuItems, 2, 0 };
}

// Calculate x position of text to centre it on screen
int Menu::xTextCentre(const char *text, int fontCharWidth) {
  // +1 to include blank space pixel on right edge of final character
  return (DISPLAY_WIDTH - (strlen(text) * fontCharWidth)) / 2 + 1;
}
