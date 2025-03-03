#include "battery.h"

// Read reference pin for what microcontroller considers 3.3v
// Compare battery pin to this to get battery voltage reading
// Taken from https://rntlab.com/question/the-adc-seems-terrible/
int getBatteryVoltage(uint8_t batteryPin, uint8_t batteryReferencePin) {
  // Multiples reads for average
  int batterySum = 0;
  int referenceSum = 0;
  for (int i = 0; i < 10; i++) {
    batterySum += analogRead(batteryPin);
    referenceSum += analogRead(batteryReferencePin);
    delay(1);
  }
  int batteryRaw = batterySum / 10;
  int referenceRaw = referenceSum / 10;

  // Convert ADC reading to voltage
  // Use 3.3v rail as reference
  int voltage = (batteryRaw * DESIRED_REFERENCE_VOLTAGE) / referenceRaw;

  return voltage;
}
