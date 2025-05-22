#include "settings.h"

// Create settings object to store settings state
// Initialised with default settings
Settings settings;

void setup() {
  // Setup
  Serial.begin(115200);

  Serial.println("Loading...");
  // Load settings from non-volatile memory
  settings.loadSettingsStorage();

  // Allow for serial to connect
  delay(200);

  Serial.printf("%i, %i\n", settings.scanIntervalIndex.get(), settings.scanInterval.get());
  Serial.printf("%i, %i\n", settings.buzzerIndex.get(), settings.buzzer.get());
  Serial.printf("%i, %i\n", settings.batteryAlarmIndex.get(), settings.batteryAlarm.get());
  Serial.printf("%i, %i\n", settings.lowCalibratedRssi.get(), settings.highCalibratedRssi.get());

  Serial.println("Setting...");
  // Set test settings
  settings.scanIntervalIndex.set(3);
  settings.buzzerIndex.set(2);
  settings.batteryAlarmIndex.set(2);
  settings.lowCalibratedRssi.set(1000);
  settings.highCalibratedRssi.set(2000);

  Serial.printf("%i, %i\n", settings.scanIntervalIndex.get(), settings.scanInterval.get());
  Serial.printf("%i, %i\n", settings.buzzerIndex.get(), settings.buzzer.get());
  Serial.printf("%i, %i\n", settings.batteryAlarmIndex.get(), settings.batteryAlarm.get());
  Serial.printf("%i, %i\n", settings.lowCalibratedRssi.get(), settings.highCalibratedRssi.get());
}

void loop() {
  // put your main code here, to run repeatedly:
}
