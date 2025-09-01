#include "api.h"
#include "battery.h"
#include "buzzer.h"
#include "menu.h"
#include "RX5808.h"
#include "settings.h"

// Create settings object to store settings state
// Initialised with default settings
Settings settings;

// Create buzzer object
Buzzer buzzer(BUZZER_PIN);

// Create battery object
Battery battery(BATTERY_PIN, &settings);

// Create RX5808 object
RX5808 module(SPI_DATA_PIN, SPI_LE_PIN, SPI_CLK_PIN, RSSI_PIN, &settings);

// Create api object
Api api(&settings, &module, &battery);

// Create menu object
Menu menu(PREVIOUS_BUTTON_PIN, SELECT_BUTTON_PIN, NEXT_BUTTON_PIN, &settings, &buzzer, &module, &api);

void setup() {
  // Setup serial for debugging
  Serial.begin(115200);

  // Load settings from non-volatile memory
  settings.loadSettingsStorage();

  // Setup menu
  menu.begin();

  // Double buzz for initialisation complete
  buzzer.doubleBuzz();

  // Allow for serial to connect
  delay(200);
}

void loop() {
  // Update battery voltage each loop
  battery.updateBatteryVoltage();

  // Start battery alarm if low voltage
  if (battery.lowBattery()) {
    buzzer.startAlarm();
  } else {
    buzzer.stopAlarm();
  }

  // Handle button presses
  // Menu object internally stores which menu currently on
  menu.handleButtons();

  // Clear display buffer
  menu.clearBuffer();

  // Draw menus using internal menu and settings states
  menu.drawMenu();

  // Draw battery voltage
  menu.drawBatteryVoltage(battery.currentVoltage.get());

  // Send display buffer
  menu.sendBuffer();
}
