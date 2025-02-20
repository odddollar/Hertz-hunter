#include <U8g2lib.h>
#include "bitmaps.h"

// Version information
const char *version = "v0.1.0";

// Define non-standard (i.e. non-i2c and non-spi) pins
#define PREVIOUS_BUTTON 21
#define SELECT_BUTTON 20
#define NEXT_BUTTON 10

// Number of ms to delay for debouncing buttons
#define DEBOUNCE_DELAY 200

// How long button has to be held to be long-pressed
#define LONG_PRESS_DURATION 300

// Used to handle long-pressing SELECT to go back
unsigned long selectButtonPressTime = 0;
bool selectButtonHeld = false;

// Setup display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Single menu item with icon
struct menuItemStruct {
  const char *name;
  const unsigned char *icon;
};

// Whole menu with multiple items
struct menuStruct {
  const char *name;
  const menuItemStruct *menuItems;
  const int menuItemsLength;
  int menuIndex;
};

// Create items in main menu
const menuItemStruct mainMenuItems[] = {
  { "Scan", bitmap_Scan },
  { "Settings", bitmap_Settings },
  { "About", bitmap_About }
};

// Create items in settings menu
// TODO: Update icons
const menuItemStruct settingsMenuItems[] = {
  { "Scan interval", bitmap_Scan },
  { "Buzzer", bitmap_Buzzer },
  { "Bat. alarm", bitmap_Alarm }
};

// Struct containing all menus
int menusIndex = 0;
menuStruct menus[] = {
  { "Main", mainMenuItems, 3, 0 },
  { "Scan", nullptr, 60, 0 },  // 60 frequencies to scan. TODO: Make dynamic with scan interval setting
  { "Settings", settingsMenuItems, 3, 0 },
  { "About", nullptr, 1, 0 }  // Given length of 1 to prevent zero-division
};

void drawMainMenu() {
  // Clear screen
  u8g2.clearBuffer();

  // Draw title
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(16, 13, "Hertz Hunter");
  u8g2.setFont(u8g2_font_7x13_tf);

  // Draw menu items
  for (int i = 0; i < menus[0].menuItemsLength; i++) {
    if (i == menus[0].menuIndex) {
      // Highlight selection
      u8g2.drawBox(0, 16 + (i * 16), 128, 16);
      u8g2.setDrawColor(0);
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menus[0].menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menus[0].menuItems[i].name);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menus[0].menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menus[0].menuItems[i].name);
    }
  }

  // Send drawing to display
  u8g2.sendBuffer();
}

// TODO: Update with settings options
void drawSettingsMenu() {
  // Clear screen
  u8g2.clearBuffer();

  // Draw title
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(32, 13, "Settings");
  u8g2.setFont(u8g2_font_7x13_tf);

  // Draw menu items
  for (int i = 0; i < menus[2].menuItemsLength; i++) {
    if (i == menus[2].menuIndex) {
      // Highlight selection
      u8g2.drawBox(0, 16 + (i * 16), 128, 16);
      u8g2.setDrawColor(0);
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menus[2].menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menus[2].menuItems[i].name);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, menus[2].menuItems[i].icon);
      u8g2.drawStr(30, 28 + (i * 16), menus[2].menuItems[i].name);
    }
  }

  // Send drawing to display
  u8g2.sendBuffer();
}

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

  // Allow for serial to connect
  delay(200);
}

void loop() {
  // Draw appropriate menu each loop
  if (menusIndex == 0) {
    drawMainMenu();
  } else if (menusIndex == 2) {
    drawSettingsMenu();
  } else if (menusIndex == 3) {
    drawAboutMenu();
  } else {
    u8g2.clearDisplay();
  }

  // Move between menu items
  if (digitalRead(NEXT_BUTTON) == HIGH) {
    menus[menusIndex].menuIndex = (menus[menusIndex].menuIndex + 1) % menus[menusIndex].menuItemsLength;
    Serial.printf("%i %i\n", menusIndex, menus[menusIndex].menuIndex);
    delay(DEBOUNCE_DELAY);  // Debounce
  } else if (digitalRead(PREVIOUS_BUTTON) == HIGH) {
    menus[menusIndex].menuIndex = (menus[menusIndex].menuIndex - 1 + menus[menusIndex].menuItemsLength) % menus[menusIndex].menuItemsLength;
    Serial.printf("%i %i\n", menusIndex, menus[menusIndex].menuIndex);
    delay(DEBOUNCE_DELAY);  // Debounce
  }

  // Handle pressing and holding select button to go back
  if (digitalRead(SELECT_BUTTON) == HIGH) {
    if (selectButtonPressTime == 0) {
      // Button just pressed so record time
      selectButtonPressTime = millis();
    } else if (!selectButtonHeld && millis() - selectButtonPressTime > LONG_PRESS_DURATION) {
      // If held for longer than threshold, register long press
      // Go back to main menu
      menusIndex = 0;
      selectButtonHeld = true;
    }
    delay(DEBOUNCE_DELAY);
  } else {
    // If button was pressed but wasn't held then use as SELECT rather than BACK
    if (selectButtonPressTime > 0 && !selectButtonHeld) {
      // Handle select on main menu
      if (menusIndex == 0) {
        if (menus[0].menuIndex == 0) {  // Go to scan menu
          menusIndex = 1;
        } else if (menus[0].menuIndex == 1) {  // Go to settings menu
          menusIndex = 2;
        } else if (menus[0].menuIndex == 2) {  // Go to about menu
          menusIndex = 3;
        }
      }
    }

    // Reset when button released
    selectButtonPressTime = 0;
    selectButtonHeld = false;
  }
}
