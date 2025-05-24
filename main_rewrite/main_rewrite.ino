#include "battery.h"
#include "buzzer.h"
#include "pins.h"
#include "settings.h"

// Create settings object to store settings state
// Initialised with default settings
Settings settings;

// Create buzzer object
Buzzer buzzer(BUZZER_PIN);

// Create battery object
Battery battery(BATTERY_PIN, &settings);

void setup() {
  // Setup serial for debugging
  Serial.begin(115200);

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
}
