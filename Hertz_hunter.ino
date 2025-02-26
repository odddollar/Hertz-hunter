#include "menu.h"
#include "storage.h"
#include "RX5808.h"

// Define button pins
#define PREVIOUS_BUTTON 21
#define SELECT_BUTTON 20
#define NEXT_BUTTON 10

// Define spi pins
#define SPI_DATA 6
#define SPI_LE 7
#define SPI_CLK 4

// Define rssi pin
#define RSSI 3

// Number of ms to delay for debouncing buttons
#define DEBOUNCE_DELAY 200

// How long button has to be held to be long-pressed (plus debounce delay)
#define LONG_PRESS_DURATION 300

// Used to handle long-pressing SELECT to go back
unsigned long selectButtonPressTime = 0;
bool selectButtonHeld = false;

// Keep track of current menu
int menusIndex = 0;

// Keep track of settings
// Scan interval settings { 5, 10, 20 }
// Buzzer settings { On, Off }
// Battery alarm settings { 3.6, 3.3, 3.0 }
int settingsIndices[] = { 0, 0, 0 };

// RX5808 module
RX5808 module(SPI_DATA, SPI_LE, SPI_CLK, RSSI);

bool justScanned = false;

// Keep track of how many frequencies need to be scanned and their values
int numFrequenciesToScan = 0;
int *frequencyValues = nullptr;

void setup() {
  // Setup
  u8g2.begin();
  Serial.begin(115200);

  // Setup button pins
  pinMode(PREVIOUS_BUTTON, INPUT_PULLDOWN);
  pinMode(SELECT_BUTTON, INPUT_PULLDOWN);
  pinMode(NEXT_BUTTON, INPUT_PULLDOWN);

  // Create preferences namespace
  preferences.begin("settings", false);

  // Load settings from non-volatile memory
  readSettingsStorage(settingsIndices);

  // Update each settings menu's icons
  for (int i = 0; i < 3; i++) {
    updateMenuIcons(&menus[i + 5], settingsIndices[i]);
  }

  // 300MHz from 5645MHz to 5945MHz
  // Interval of 5MHz is 60 frequencies to scan
  // Interval of 10MHz is 30 frequencies to scan
  // Interval of 20MHz is 15 frequencies to scan
  numFrequenciesToScan = 300 / (5 * pow(2, settingsIndices[0]));
  menus[1].menuItemsLength = numFrequenciesToScan;

  // Allocate space for scanned rssi values
  frequencyValues = (int*)malloc(numFrequenciesToScan * sizeof(int));

  // Allow for serial to connect
  delay(200);
}

void loop() {
  // Draw appropriate menu
  // Only scan (menus index 1) and about (menus index 3) need unique functions
  if (menusIndex == 1) {
    u8g2.clearDisplay();

    if (!justScanned) {
      // Scan frequencies and print
      for (int i = 0; i <= 60; i++) {
        module.setFrequency(i * 5 + 5645);
        delay(30);
        Serial.printf("%i ", module.readRSSI());
      }
      Serial.println();
      justScanned = true;
    }
  } else if (menusIndex == 3) {
    drawAboutMenu();
    justScanned = false;
  } else {
    drawSelectionMenu(&menus[menusIndex]);
    justScanned = false;
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
      writeSettingsStorage(settingsIndices);
      updateMenuIcons(&menus[menusIndex], menus[menusIndex].menuIndex);

      // If scan interval changed update scan menu length
      if (menusIndex == 5) {
        numFrequenciesToScan = 300 / (5 * pow(2, settingsIndices[0]));
        menus[1].menuItemsLength = numFrequenciesToScan;

        // Reallocate space for scanned rssi values
        frequencyValues = (int*)realloc(frequencyValues, numFrequenciesToScan * sizeof(int));
      }
    }
  }

  // Reset select when button released
  selectButtonPressTime = 0;
  selectButtonHeld = false;
}
