#include <U8g2lib.h>
#include "bitmaps.h"

// Define non-standard (i.e. non-i2c and non-spi) pins
#define PREVIOUS_BUTTON 21
#define SELECT_BUTTON 20
#define NEXT_BUTTON 10

// Number of ms to delay for debouncing buttons
#define DEBOUNCE_DELAY 200

// How long button has to be held to be long-pressed
#define LONG_PRESS_DURATION 800

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
const menuItemStruct settingsMenuItems[] = {
  { "Scan interval", bitmap_Scan }
};

// Struct containing all menus
int menusIndex = 0;
menuStruct menus[] = {
  { "Main", mainMenuItems, 3, 0 },
  { "Scan", nullptr, 60, 0 },  // 60 frequencies to scan
  { "Settings", settingsMenuItems, 1, 0 },
  { "About", nullptr, 1, 0 }  // Given lenght of 1 to prevent zero-division
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
  // Draw main menu each loop
  if (menusIndex == 0) {
    drawMainMenu();
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
      Serial.println("Back");
      menusIndex = 0;
      selectButtonHeld = true;
    }
    delay(DEBOUNCE_DELAY);
  } else {
    // If button was pressed but wasn't held then use as SELECT rather than BACK
    if (selectButtonPressTime > 0 && !selectButtonHeld) {
      if (menus[0].menuIndex == 0) {  // Go to scan menu
        Serial.println("Scan");
        menusIndex = 1;
      } else if (menus[0].menuIndex == 1) {  // Go to settings menu
        Serial.println("Settings");
        menusIndex = 2;
      } else if (menus[0].menuIndex == 2) {  // Go to about menu
        Serial.println("About");
        menusIndex = 3;
      }
    }

    // Reset when button released
    selectButtonPressTime = 0;
    selectButtonHeld = false;
  }
}
