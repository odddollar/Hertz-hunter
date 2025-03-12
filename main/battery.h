#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

#define BATTERY_VOLTAGE_OFFSET 1

int getBatteryVoltage(uint8_t batteryPin);

#endif
