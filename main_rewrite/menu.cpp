#include "menu.h"

Menu::Menu(uint8_t p_p, uint8_t s_p, uint8_t n_p, Settings *s, Buzzer *b)
  : menuIndex(MAIN),
    previous_pin(p_p), select_pin(s_p), next_pin(n_p),
    selectButtonPressTime(0), selectButtonHeld(false),
    settings(s), buzzer(b),
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

  // Move between menu items
  if (nextPressed == HIGH || prevPressed == HIGH) {
    int direction = (nextPressed == HIGH) ? 1 : -1;
    menus[menuIndex].menuIndex = (menus[menuIndex].menuIndex + direction + menus[menuIndex].menuItemsLength) % menus[menuIndex].menuItemsLength;

    // Sound buzzer on button press if necessary
    if (settings->buzzer.get()) buzzer->buzz();
  }

  // Handle pressing and holding SELECT to go back
  if (selectPressed == HIGH) {
    if (selectButtonPressTime == 0) {  // Button just pressed so record time
      selectButtonPressTime = millis();
    } else if (!selectButtonHeld && millis() - selectButtonPressTime > LONG_PRESS_DURATION) {  // Held longer than threshold register long press
      switch (menuIndex) {
        case MAIN: menuIndex = ADVANCED; break;                             // If on main menu, go to advanced
        case SCAN_INTERVAL ... BATTERY_ALARM: menuIndex = SETTINGS; break;  // If on individual settings menu, go to settings
        case WIFI ... CALIBRATION: menuIndex = ADVANCED; break;             // If on individual advanced menu, go to advanced
        default: menuIndex = MAIN; break;                                   // Otherwise, go back to main menu
      }

      selectButtonHeld = true;
      if (settings->buzzer.get()) buzzer->doubleBuzz();
    }

    // Immediately end and wait for next iteration of loop()
    return;
  }

  // Reset SELECT when button released
  selectButtonPressTime = 0;
  selectButtonHeld = false;

  // Delay for button debouncing
  delay(DEBOUNCE_DELAY);
}

// Draw current menu
void Menu::drawMenu(int voltage) {
  // Clear screen
  u8g2.clearBuffer();

  // Calculate x position of title
  // 8 is width of font char
  // +1 to include blank space pixel on right edge of final character
  int titleXPos = (DISPLAY_WIDTH - (strlen(menus[menuIndex].title) * 8)) / 2 + 1;

  // Draw title, but not for scan menu
  if (menuIndex != SCAN) {
    u8g2.setFont(u8g2_font_8x13B_tf);
    u8g2.drawStr(titleXPos, 13, menus[menuIndex].title);
    u8g2.setFont(u8g2_font_7x13_tf);
  }

  // Draw voltage only display if on main menu
  if (menuIndex == MAIN) {
    drawBatteryVoltage(voltage);
  }

  // Send drawing to display
  u8g2.sendBuffer();
}

// Display battery voltage in bottom corner of main menu
void Menu::drawBatteryVoltage(int voltage) {
  // Format voltage reading
  char formattedVoltage[5];
  snprintf(formattedVoltage, sizeof(formattedVoltage), "%d.%dv", voltage / 10, voltage % 10);

  // Set font colour to inverted if selected bottom item
  u8g2.setDrawColor(menuIndex == 2 ? 0 : 1);
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(109, DISPLAY_HEIGHT, formattedVoltage);
  u8g2.setDrawColor(1);
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
