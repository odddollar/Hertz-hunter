#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

#define DESIRED_REFERENCE_VOLTAGE 33

int getBatteryVoltage(uint8_t batteryPin, uint8_t batteryReferencePin);

#endif
