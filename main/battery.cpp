#include "battery.h"

Battery::Battery(uint8_t p, Settings* s)
  : pin(p), lastLowBatteryTime(0), settings(s) {

  // Setup battery input pin
  pinMode(pin, INPUT);

  // Create mutex
  batteryMutex = xSemaphoreCreateMutex();
}

// Update internal battery voltage state
// Accounts for voltage divider
void Battery::updateBatteryVoltage() {
  // Multiple reads for average
  int raw = 0;
  for (int i = 0; i < 10; i++) {
    raw += analogReadMilliVolts(pin);
  }
  raw /= 10;

  // Format voltage
  int formatted = round(raw / 100.0 * 2) + BATTERY_VOLTAGE_OFFSET;

  xSemaphoreTake(batteryMutex, portMAX_DELAY);
  currentVoltage.set(formatted);
  xSemaphoreGive(batteryMutex);
}

// Battery below alarm threshold for long enough to be considered "low"
bool Battery::lowBattery() {
  xSemaphoreTake(batteryMutex, portMAX_DELAY);
  int voltage = currentVoltage.get();
  xSemaphoreGive(batteryMutex);

  // Safely get value
  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  int threshold = settings->batteryAlarm.get();
  xSemaphoreGive(settings->settingsMutex);

  if (voltage <= threshold && lastLowBatteryTime == 0) {
    lastLowBatteryTime = millis();
  } else if (voltage <= threshold && millis() - lastLowBatteryTime > MIN_LOW_BATTERY_TIME) {
    return true;
  } else if (voltage > threshold) {
    lastLowBatteryTime = 0;
  }

  return false;
}
