#include <U8g2lib.h>
#include <Preferences.h>
#include "bitmaps.h"
#include "menu.h"

// Version information
const char *version = "v0.1.0";

// Define button pins
#define PREVIOUS_BUTTON 21
#define SELECT_BUTTON 20
#define NEXT_BUTTON 10

// Number of ms to delay for debouncing buttons
#define DEBOUNCE_DELAY 200

// How long button has to be held to be long-pressed (plus debounce delay)
#define LONG_PRESS_DURATION 300

// Used to handle long-pressing SELECT to go back
unsigned long selectButtonPressTime = 0;
bool selectButtonHeld = false;

// Setup display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Keep track of current menu
int menusIndex = 0;

// Preferences for storing data to non-volatile memory
Preferences preferences;

// Keep track of settings
// Scan interval possibilities { 5, 10, 20 }
// Buzzer settings { On, Off }
// Battery alarm settings { 3.6, 3.3, 3.0 }
int settingsIndices[] = { 0, 0, 0 };

// Write settings to non-volatile memory
void writeSettingsStorage() {
  for (int i = 0; i < 3; i++) {
    preferences.putInt(String(i).c_str(), settingsIndices[i]);
  }
}

// Read settings from non-volatile memory
void readSettingsStorage() {
  for (int i = 0; i < 3; i++) {
    settingsIndices[i] = preferences.getInt(String(i).c_str(), 0);
  }
}

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
  u8g2.drawStr(15, 60, "By Simon Eason");

  // Send drawing to display
  u8g2.sendBuffer();
}

void setup() {
  // Setup
  u8g2.begin();
  Serial.begin(115200);

  // Setup pins
  pinMode(PREVIOUS_BUTTON, INPUT_PULLDOWN);
  pinMode(SELECT_BUTTON, INPUT_PULLDOWN);
  pinMode(NEXT_BUTTON, INPUT_PULLDOWN);

  // Create preferences namespace
  preferences.begin("settings", false);

  // Load settings from non-volatile memory
  readSettingsStorage();

  // Update each settings menu's icons
  for (int i = 0; i < 3; i++) {
    updateMenuIcons(&menus[i + 5], settingsIndices[i]);
  }

  // Allow for serial to connect
  delay(200);
}

void loop() {
  // Draw appropriate menu
  // Only scan (menus index 1) and about (menus index 3) need unique functions
  if (menusIndex == 1) {
    u8g2.clearDisplay();
  } else if (menusIndex == 3) {
    drawAboutMenu();
  } else {
    drawSelectionMenu(&menus[menusIndex]);
  }

  // Move between menu items
  if (digitalRead(NEXT_BUTTON) == HIGH) {
    menus[menusIndex].menuIndex = (menus[menusIndex].menuIndex + 1) % menus[menusIndex].menuItemsLength;
    delay(DEBOUNCE_DELAY);  // Debounce
  } else if (digitalRead(PREVIOUS_BUTTON) == HIGH) {
    menus[menusIndex].menuIndex = (menus[menusIndex].menuIndex - 1 + menus[menusIndex].menuItemsLength) % menus[menusIndex].menuItemsLength;
    delay(DEBOUNCE_DELAY);  // Debounce
  }

  // Handle pressing and holding select button to go back
  if (digitalRead(SELECT_BUTTON) == HIGH) {
    if (selectButtonPressTime == 0) {  // Button just pressed so record time
      selectButtonPressTime = millis();
    } else if (!selectButtonHeld && millis() - selectButtonPressTime > LONG_PRESS_DURATION) {  // Held longer than threshold register long press
      // If on main menu go to calibration
      if (menusIndex == 0) {
        menusIndex = 4;
      } else if (menusIndex >= 5) {  // If on individual option go to settings
        menusIndex = 2;
      } else {  // Otherwise go back to main
        menusIndex = 0;
      }
      selectButtonHeld = true;
    }
    delay(DEBOUNCE_DELAY);

    // Immediately complete loop
    return;
  }

  // If select button was pressed but wasn't held then use as SELECT rather than BACK
  if (selectButtonPressTime > 0 && !selectButtonHeld) {
    if (menusIndex == 0) {            // Handle select on main menu
      if (menus[0].menuIndex == 0) {  // Go to scan menu
        menusIndex = 1;
      } else if (menus[0].menuIndex == 1) {  // Go to settings menu
        menusIndex = 2;
      } else if (menus[0].menuIndex == 2) {  // Go to about menu
        menusIndex = 3;
      }
    } else if (menusIndex == 2) {     // Handle select on settings menu
      if (menus[2].menuIndex == 0) {  // Go to scan interval menu
        menusIndex = 5;
      } else if (menus[2].menuIndex == 1) {  // Go to buzzer menu
        menusIndex = 6;
      } else if (menus[2].menuIndex == 2) {  // Go to battery alarm menu
        menusIndex = 7;
      }
    } else if (menusIndex >= 5) {  // Handle select on individual options
      settingsIndices[menusIndex - 5] = menus[menusIndex].menuIndex;
      writeSettingsStorage();
      updateMenuIcons(&menus[menusIndex], menus[menusIndex].menuIndex);
    }
  }

  // Reset select when button released
  selectButtonPressTime = 0;
  selectButtonHeld = false;
}
