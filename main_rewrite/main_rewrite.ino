#include "battery.h"
#include "buzzer.h"
#include "menu.h"
#include "pins.h"
#include "settings.h"

// Create settings object to store settings state
// Initialised with default settings
Settings settings;

// Create buzzer object
Buzzer buzzer(BUZZER_PIN);

// Create battery object
Battery battery(BATTERY_PIN, &settings);

// Create menu object
Menu menu(PREVIOUS_BUTTON_PIN, SELECT_BUTTON_PIN, NEXT_BUTTON_PIN, &settings, &buzzer, &battery);

void setup() {
  // Setup serial for debugging
  Serial.begin(115200);

  // Setup menu
  menu.begin();

  // Load settings from non-volatile memory
  settings.loadSettingsStorage();

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

  // Draw menus using internal menu and settings states
  menu.drawMenu();
}
