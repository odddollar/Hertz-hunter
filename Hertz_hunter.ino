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

// Menu state
int menuIndex = 0;
const char* menuItems[] = { "Scan", "Settings", "About" };
const int menuLength = sizeof(menuItems) / sizeof(menuItems[0]);

void drawMenu() {
  // Clear screen
  u8g2.clearBuffer();

  // Draw title
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(16, 13, "Hertz Hunter");
  u8g2.setFont(u8g2_font_7x13_tf);

  // Draw menu items
  for (int i = 0; i < menuLength; i++) {
    if (i == menuIndex) {
      // Highlight selection
      u8g2.drawBox(0, 16 + (i * 16), 128, 16);
      u8g2.setDrawColor(0);
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, epd_bitmap_allArray[i]);
      u8g2.drawStr(30, 28 + (i * 16), menuItems[i]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawXBMP(10, 17 + (i * 16), 14, 14, epd_bitmap_allArray[i]);
      u8g2.drawStr(30, 28 + (i * 16), menuItems[i]);
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
  // Draw menu each loop based on global state
  drawMenu();

  // Move between menu items
  if (digitalRead(NEXT_BUTTON) == HIGH) {
    menuIndex = (menuIndex + 1) % menuLength;
    delay(DEBOUNCE_DELAY);  // Debounce
  } else if (digitalRead(PREVIOUS_BUTTON) == HIGH) {
    menuIndex = (menuIndex - 1 + menuLength) % menuLength;
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
      selectButtonHeld = true;
    }
    delay(DEBOUNCE_DELAY);
  } else {
    // If button was pressed but wasn't held then use as SELECT rather than BACK
    if (selectButtonPressTime > 0 && !selectButtonHeld) {
      if (menuIndex == 0) {
        Serial.println("Scan");
      } else if (menuIndex == 1) {
        Serial.println("Settings");
      } else if (menuIndex == 2) {
        Serial.println("About");
      }
    }

    // Reset when button released
    selectButtonPressTime = 0;
    selectButtonHeld = false;
  }
}
