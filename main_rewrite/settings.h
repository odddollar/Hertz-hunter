#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "variable.h"
#include "callback_variable.h"

#define DEFAULT_INDEX 0
#define DEFAULT_SCAN_INTERVAL 5
#define DEFAULT_BUZZER true
#define DEFAULT_BATTERY_ALARM 36

// Holds the state for the settings and handles updates to options
class Settings {
public:
  Settings();

  CallbackVariable<int> scanIntervalIndex;
  Variable<int> scanInterval;  // Should not be directly set
  CallbackVariable<int> buzzerIndex;
  Variable<bool> buzzer;  // Should not be directly set
  CallbackVariable<int> batteryAlarmIndex;
  Variable<int> batteryAlarm;  // Should not be directly set
};

#endif
