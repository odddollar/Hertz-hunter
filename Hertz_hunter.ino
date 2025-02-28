#include "menu.h"
#include "storage.h"
#include "module.h"

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
#define DEBOUNCE_DELAY 150

// How long button has to be held to be long-pressed (plus debounce delay)
#define LONG_PRESS_DURATION 350

// Maximum number of frequencies to be scanned
// + 1 to include final frequency
#define MAX_NUMBER_FREQUENCIES 60 + 1

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

// Hold calibrated rssi values in form { low, high }
int calibratedRssi[] = { 0, 0 };

// RX5808 module
RX5808 module(SPI_DATA, SPI_LE, SPI_CLK, RSSI);

// Keep track of how many frequencies need to be scanned and their values
// 60 is 300 / 5, the smallest scanning interval
int numFrequenciesToScan = MAX_NUMBER_FREQUENCIES;
int scanInterval = 5;
int rssiValues[MAX_NUMBER_FREQUENCIES];

// Mutex to prevent scanning and drawing from accessing same data simultaneously
SemaphoreHandle_t mutex;

// Task handle to allow starting and stopping of scanning task/thread
TaskHandle_t scanTaskHandle = NULL;

// Run scanning function continuously
// Will be run in separate task and task will be cancelled when unneeded
void scanContinuously(void *parameter) {
  while (1) {
    module.scan(rssiValues, numFrequenciesToScan, 5645, scanInterval, mutex);
  }
}

// Delete handle for scanning task
void stopScanContinuously() {
  if (scanTaskHandle != NULL) {
    vTaskDelete(scanTaskHandle);
    scanTaskHandle = NULL;
  }
}

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

  // Load calibration from non-volatile memory
  readCalibrationStorage(calibratedRssi);

  // Update each settings menu's icons
  for (int i = 0; i < 3; i++) {
    updateMenuIcons(&menus[i + 5], settingsIndices[i]);
  }

  // 300MHz from 5645MHz to 5945MHz
  // Interval of 5MHz is 60 frequencies to scan
  // Interval of 10MHz is 30 frequencies to scan
  // Interval of 20MHz is 15 frequencies to scan
  scanInterval = 5 * pow(2, settingsIndices[0]);
  numFrequenciesToScan = (300 / scanInterval) + 1;  // + 1 for final number inclusion
  menus[1].menuItemsLength = numFrequenciesToScan;

  // Fill entirety of rssi values array with 0
  for (int i = 0; i < MAX_NUMBER_FREQUENCIES; i++) {
    rssiValues[i] = 0;
  }

  // Create mutex
  mutex = xSemaphoreCreateMutex();

  // Allow for serial to connect
  delay(200);
}

void loop() {
  // Draw the appropriate menu
  switch (menusIndex) {
    case 1:
      // Start scanning in new thread
      if (scanTaskHandle == NULL) {
        xTaskCreate(
          scanContinuously,
          "ScanContinuously",
          2048,
          NULL,
          1,
          &scanTaskHandle);
      }

      drawScanMenu(&menus[1], rssiValues, numFrequenciesToScan, 5645, scanInterval, calibratedRssi, mutex);
      break;
    case 3:
      stopScanContinuously();
      drawAboutMenu();
      break;
    default:
      stopScanContinuously();
      drawSelectionMenu(&menus[menusIndex]);
      break;
  }

  // Check menu button presses
  int nextPressed = digitalRead(NEXT_BUTTON);
  int prevPressed = digitalRead(PREVIOUS_BUTTON);

  // Move between menu items
  if (nextPressed == HIGH || prevPressed == HIGH) {
    int direction = (nextPressed == HIGH) ? 1 : -1;
    menus[menusIndex].menuIndex = (menus[menusIndex].menuIndex + direction + menus[menusIndex].menuItemsLength) % menus[menusIndex].menuItemsLength;
    delay(DEBOUNCE_DELAY);  // Debounce
  }

  // Handle pressing and holding select button to go back
  if (digitalRead(SELECT_BUTTON) == HIGH) {
    if (selectButtonPressTime == 0) {
      // Button just pressed so record time
      selectButtonPressTime = millis();
    } else if (!selectButtonHeld && millis() - selectButtonPressTime > LONG_PRESS_DURATION) {
      // Held longer than threshold register long press
      switch (menusIndex) {
        case 0: menusIndex = 4; break;          // If on main menu, go to calibration
        case 5 ... 255: menusIndex = 2; break;  // If on an individual option, go to settings
        default: menusIndex = 0; break;         // Otherwise, go back to the main menu
      }
      selectButtonHeld = true;
    }
    delay(DEBOUNCE_DELAY);

    // Immediately complete loop
    return;
  }

  // If select button was pressed but not held, use as SELECT rather than BACK
  if (selectButtonPressTime > 0 && !selectButtonHeld) {
    switch (menusIndex) {
      case 0:  // Handle select on main menu
        switch (menus[0].menuIndex) {
          case 0: menusIndex = 1; break;  // Go to scan menu
          case 1: menusIndex = 2; break;  // Go to settings menu
          case 2: menusIndex = 3; break;  // Go to about menu
        }
        break;
      case 2:  // Handle select on settings menu
        switch (menus[2].menuIndex) {
          case 0: menusIndex = 5; break;  // Go to scan interval menu
          case 1: menusIndex = 6; break;  // Go to buzzer menu
          case 2: menusIndex = 7; break;  // Go to battery alarm menu
        }
        break;
      case 5 ... 255:  // Handle select on individual options (menusIndex >= 5)
        settingsIndices[menusIndex - 5] = menus[menusIndex].menuIndex;
        writeSettingsStorage(settingsIndices);
        updateMenuIcons(&menus[menusIndex], menus[menusIndex].menuIndex);

        // If scan interval changed, update scan menu length
        if (menusIndex == 5) {
          scanInterval = 5 * pow(2, settingsIndices[0]);
          numFrequenciesToScan = (300 / scanInterval) + 1;  // +1 for final number inclusion
          menus[1].menuItemsLength = numFrequenciesToScan;
          menus[1].menuIndex = 0;
        }

        break;
    }
  }

  // Reset select when button released
  selectButtonPressTime = 0;
  selectButtonHeld = false;
}
