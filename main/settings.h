#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>
#include "esp_system.h"
#include "variable.h"

#define DEFAULT_INDEX 0
#define DEFAULT_SCAN_INTERVAL 2.5
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

  VariableCallback<int> scanIntervalIndex;
  VariableRestricted<float> scanInterval;  // Should not be directly set outside class
  VariableCallback<int> buzzerIndex;
  VariableRestricted<bool> buzzer;  // Should not be directly set outside class
  VariableCallback<int> batteryAlarmIndex;
  VariableRestricted<int> batteryAlarm;  // Should not be directly set outside class
  VariableCallback<int> lowCalibratedRssi;
  VariableCallback<int> highCalibratedRssi;

private:
  bool initialReadDone;
  
  Preferences preferences;
};

#endif
