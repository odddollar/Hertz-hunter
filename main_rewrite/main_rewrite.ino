#include "settings.h"

// Create settings object to store settings state
// Initialised with default settings
Settings settings;

void setup() {
  // Setup
  Serial.begin(115200);

  // Allow for serial to connect
  delay(200);

  Serial.printf("%i, %i\n", settings.scanIntervalIndex.get(), settings.scanInterval.get());
  Serial.printf("%i, %i\n", settings.buzzerIndex.get(), settings.buzzer.get());
  Serial.printf("%i, %i\n", settings.batteryAlarmIndex.get(), settings.batteryAlarm.get());

  // Set test settings
  settings.scanIntervalIndex.set(2);
  settings.buzzerIndex.set(1);
  settings.batteryAlarmIndex.set(1);

  Serial.printf("%i, %i\n", settings.scanIntervalIndex.get(), settings.scanInterval.get());
  Serial.printf("%i, %i\n", settings.buzzerIndex.get(), settings.buzzer.get());
  Serial.printf("%i, %i\n", settings.batteryAlarmIndex.get(), settings.batteryAlarm.get());
}

void loop() {
  // put your main code here, to run repeatedly:
}
