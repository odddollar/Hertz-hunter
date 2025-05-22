#include "settings.h"

// Create settings object to store settings state
// Initialised with default settings
Settings settings;

void setup() {
  // Setup
  Serial.begin(115200);

  // Load settings from non-volatile memory
  settings.loadSettingsStorage();

  // Allow for serial to connect
  delay(200);
}

void loop() {
  // put your main code here, to run repeatedly:
}
