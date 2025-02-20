#include <U8g2lib.h>

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

// "Wifi", 14x14px
const unsigned char epd_bitmap_Wifi[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0x04, 0x08, 0x02, 0x10, 0xf1, 0x23, 0x08, 0x04, 0x04, 0x08,
  0xe0, 0x01, 0x10, 0x02, 0x00, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0x00, 0x00
};
// "Settings", 14x14px
const unsigned char epd_bitmap_Settings[] PROGMEM = {
  0xc4, 0x08, 0x2a, 0x15, 0x31, 0x23, 0x02, 0x10, 0x04, 0x08, 0xc6, 0x18, 0xe1, 0x21, 0xe1, 0x21,
  0xc6, 0x18, 0x04, 0x08, 0x02, 0x10, 0x31, 0x23, 0x2a, 0x15, 0xc4, 0x08
};
// "About", 14x14px
const unsigned char epd_bitmap_About[] PROGMEM = {
  0xe0, 0x01, 0x18, 0x06, 0x04, 0x08, 0xc2, 0x10, 0xc2, 0x10, 0x01, 0x20, 0x01, 0x20, 0xc1, 0x20,
  0xc1, 0x20, 0xc2, 0x10, 0xc2, 0x10, 0x04, 0x08, 0x18, 0x06, 0xe0, 0x01
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 144)
const int epd_bitmap_allArray_LEN = 3;
const unsigned char* epd_bitmap_allArray[3] = {
  epd_bitmap_Wifi,
  epd_bitmap_Settings,
  epd_bitmap_About
};

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
