#include "battery.h"

// Read battery voltage, accounting for voltage divider
int getBatteryVoltage(uint8_t batteryPin) {
  // Multiples reads for average
  int raw = 0;
  for (int i = 0; i < 10; i++) {
    raw += analogReadMilliVolts(batteryPin);
  }
  raw /= 10;

  return round(raw / 100.0 * 2) + BATTERY_VOLTAGE_OFFSET;
}
