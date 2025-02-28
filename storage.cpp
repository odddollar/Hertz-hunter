#include "storage.h"

// Preferences for storing data to non-volatile memory
Preferences preferences;

// Write settings to non-volatile memory
void writeSettingsStorage(int settings[3]) {
  for (int i = 0; i < 3; i++) {
    preferences.putInt(String(i).c_str(), settings[i]);
  }
}

// Read settings from non-volatile memory
void readSettingsStorage(int settings[3]) {
  for (int i = 0; i < 3; i++) {
    settings[i] = preferences.getInt(String(i).c_str(), DEFAULT_SETTING_INDEX);
  }
}

// Write calibration to non-volatile memory
void writeCalibrationStorage(int calibratedRssi[2]) {
  preferences.putInt(String("minCalib").c_str(), calibratedRssi[0]);
  preferences.putInt(String("maxCalib").c_str(), calibratedRssi[1]);
}

// Read calibration from non-volatile memory
void readCalibrationStorage(int calibratedRssi[2]) {
  calibratedRssi[0] = preferences.getInt(String("minCalib").c_str(), DEFAULT_MIN_CALIBRATION);
  calibratedRssi[1] = preferences.getInt(String("maxCalib").c_str(), DEFAULT_MAX_CALIBRATION);
}
