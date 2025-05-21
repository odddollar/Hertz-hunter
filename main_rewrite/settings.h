#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "variable.h"
#include "callback_variable.h"

#define DEFAULT_INDEX 0
#define DEFAULT_SCAN_INTERVAL 5
#define DEFAULT_BUZZER true
#define DEFAULT_BATTERY_ALARM 36
#define DEFAULT_LOW_CALIBRATED_RSSI 0
#define DEFAULT_HIGH_CALIBRAYED_RSSI 4095

// Holds the state for the settings and handles updates to options
class Settings {
public:
  Settings();

  CallbackVariable<int> scanIntervalIndex;  // onChange() should not be modified outside class
  Variable<int> scanInterval;               // Should not be directly set outside class
  CallbackVariable<int> buzzerIndex;        // onChange() should not be modified outside class
  Variable<bool> buzzer;                    // Should not be directly set outside class
  CallbackVariable<int> batteryAlarmIndex;  // onChange() should not be modified outside class
  Variable<int> batteryAlarm;               // Should not be directly set outside class
  Variable<int> lowCalibratedRssi;
  Variable<int> highCalibratedRssi;
};

#endif
