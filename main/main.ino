#include "menu.h"
#include "storage.h"
#include "module.h"
#include "calibration.h"
#include "buzzer.h"

#define PREVIOUS_BUTTON 21
#define SELECT_BUTTON 20
#define NEXT_BUTTON 10

#define SPI_DATA 6
#define SPI_LE 7
#define SPI_CLK 4

#define RSSI 3

#define BUZZER 1

// Number of ms to delay for debouncing buttons
#define DEBOUNCE_DELAY 150

// How long button has to be held to be long-pressed (plus debounce delay)
#define LONG_PRESS_DURATION 350

#define MAX_NUMBER_FREQUENCIES 60 + 1  // + 1 to include final frequency
#define DEFAULT_SCAN_INTERVAL 5
#define MIN_FREQUENCY 5645
#define SCAN_FREQUENCY_RANGE 300

#define BUZZ_DURATION 20
#define DOUBLE_BUZZ_DURATION 80
#define BUZZER_STACK_SIZE 512

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
int calibratedRssi[] = { DEFAULT_MIN_CALIBRATION, DEFAULT_MAX_CALIBRATION };

// RX5808 module
RX5808 module(SPI_DATA, SPI_LE, SPI_CLK, RSSI);

// Buzzer module
Buzzer buzzer(BUZZER);
bool shouldBuzz = true;

// Keep track of how many frequencies need to be scanned and their values
// 60 is 300 / 5, the smallest scanning interval
int numFrequenciesToScan = MAX_NUMBER_FREQUENCIES;
int scanInterval = DEFAULT_SCAN_INTERVAL;
int rssiValues[MAX_NUMBER_FREQUENCIES];

// Mutex to prevent scanning and drawing from accessing same data simultaneously
SemaphoreHandle_t mutex;

// Task handle to allow starting and stopping of scanning task/thread
TaskHandle_t scanTaskHandle = NULL;

// Run scanning function continuously
// Will be run in separate task and task will be cancelled when unneeded
void scanContinuously(void *parameter) {
  while (1) {
    module.scan(rssiValues, numFrequenciesToScan, MIN_FREQUENCY, scanInterval, mutex);
  }
}

// Delete handle for scanning task
void stopScanContinuously() {
  if (scanTaskHandle != NULL) {
    vTaskDelete(scanTaskHandle);
    scanTaskHandle = NULL;
  }
}

// Spawned in another thread to prevent blocking
void buzz(void *parameter) {
  buzzer.buzz(BUZZ_DURATION);
  vTaskDelete(NULL);
}

// Spawned in another thread to prevent blocking
void doubleBuzz(void *parameter) {
  buzzer.doubleBuzz(BUZZ_DURATION, DOUBLE_BUZZ_DURATION);
  vTaskDelete(NULL);
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
  numFrequenciesToScan = (SCAN_FREQUENCY_RANGE / scanInterval) + 1;  // + 1 for final number inclusion
  menus[1].menuItemsLength = numFrequenciesToScan;

  // Set initial buzzer state
  shouldBuzz = settingsIndices[1] == 0 ? true : false;

  // Fill entirety of rssi values array with 0
  for (int i = 0; i < MAX_NUMBER_FREQUENCIES; i++) {
    rssiValues[i] = 0;
  }

  // Create mutex
  mutex = xSemaphoreCreateMutex();

  // Double buzz for initialisation complete
  xTaskCreate(doubleBuzz, "buzz", BUZZER_STACK_SIZE, NULL, 1, NULL);

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

      drawScanMenu(&menus[1], rssiValues, numFrequenciesToScan, MIN_FREQUENCY, scanInterval, calibratedRssi, mutex);
      break;
    case 3:
      stopScanContinuously();
      drawAboutMenu(&menus[3]);
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
    if (shouldBuzz) xTaskCreate(buzz, "buzz", BUZZER_STACK_SIZE, NULL, 1, NULL);
    delay(DEBOUNCE_DELAY);  // Debounce
  }

  // Handle pressing and holding select button to go back
  if (digitalRead(SELECT_BUTTON) == HIGH) {
    if (selectButtonPressTime == 0) {
      // Button just pressed so record time
      selectButtonPressTime = millis();
      if (shouldBuzz) xTaskCreate(buzz, "buzz", BUZZER_STACK_SIZE, NULL, 1, NULL);
    } else if (!selectButtonHeld && millis() - selectButtonPressTime > LONG_PRESS_DURATION) {
      // Held longer than threshold register long press
      switch (menusIndex) {
        case 0: menusIndex = 4; break;          // If on main menu, go to calibration
        case 5 ... 255: menusIndex = 2; break;  // If on an individual option, go to settings
        default: menusIndex = 0; break;         // Otherwise, go back to the main menu
      }
      selectButtonHeld = true;
      if (shouldBuzz) xTaskCreate(doubleBuzz, "buzz", BUZZER_STACK_SIZE, NULL, 1, NULL);
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
      case 4:  // Handle select on calibration menu
        calibrateRssi(calibratedRssi, menus[4].menuIndex);
        writeCalibrationStorage(calibratedRssi);
        break;
      case 5 ... 255:  // Handle select on individual options (menusIndex >= 5)
        settingsIndices[menusIndex - 5] = menus[menusIndex].menuIndex;
        writeSettingsStorage(settingsIndices);
        updateMenuIcons(&menus[menusIndex], menus[menusIndex].menuIndex);

        // Make necessary in-memory changes for updated settings
        switch (menusIndex) {
          case 5:  // Update scan menu length
            scanInterval = 5 * pow(2, settingsIndices[0]);
            numFrequenciesToScan = (SCAN_FREQUENCY_RANGE / scanInterval) + 1;  // +1 for final number inclusion
            menus[1].menuItemsLength = numFrequenciesToScan;
            menus[1].menuIndex = 0;
            break;
          case 6:  // Update buzzer state
            shouldBuzz = settingsIndices[1] == 0 ? true : false;
            break;
        }
        break;
    }
  }

  // Reset select when button released
  selectButtonPressTime = 0;
  selectButtonHeld = false;
}
