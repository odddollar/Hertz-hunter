#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>
#include "variable.h"

#define BATTERY_VOLTAGE_OFFSET 1

// Battery monitoring for battery module
// Maintains internal store of current voltage
class Battery {
public:
  Battery(uint8_t p, Settings* s);
  void updateBatteryVoltage();
  bool lowBattery();

  VariableRestricted<int> currentVoltage;

private:
  uint8_t pin;
};

#endif
