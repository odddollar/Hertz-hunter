#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>
#include "variable.h"
#include "callback_variable.h"
#include "esp_system.h"

#define DEFAULT_INDEX 0
#define DEFAULT_SCAN_INTERVAL 5
#define DEFAULT_BUZZER true
#define DEFAULT_BATTERY_ALARM 36
#define DEFAULT_LOW_CALIBRATED_RSSI 0
#define DEFAULT_HIGH_CALIBRATED_RSSI 4095

// Holds the state for the settings and handles updates to options
class Settings {
public:
  Settings();
  void saveSettingsStorage(const char *key, int value);
  void loadSettingsStorage();
  void clearReset();

  CallbackVariable<int> scanIntervalIndex;
  Variable<int> scanInterval;  // Should not be directly set outside class
  CallbackVariable<int> buzzerIndex;
  Variable<bool> buzzer;  // Should not be directly set outside class
  CallbackVariable<int> batteryAlarmIndex;
  Variable<int> batteryAlarm;  // Should not be directly set outside class
  CallbackVariable<int> lowCalibratedRssi;
  CallbackVariable<int> highCalibratedRssi;

private:
  bool initialReadDone;
  Preferences preferences;
};

#endif
