#include "storage.h"

// Preferences for storing data to non-volatile memory
Preferences preferences;

// Write settings to non-volatile memory
void writeSettingsStorage(int settings[3]) {
  char key[2];
  for (int i = 0; i < 3; i++) {
    snprintf(key, sizeof(key), "%d", i);
    preferences.putInt(key, settings[i]);
  }
}

// Read settings from non-volatile memory
void readSettingsStorage(int settings[3]) {
  char key[2];
  for (int i = 0; i < 3; i++) {
    snprintf(key, sizeof(key), "%d", i);
    settings[i] = preferences.getInt(key, DEFAULT_SETTING_INDEX);
  }
}

// Write calibration to non-volatile memory
void writeCalibrationStorage(int calibratedRssi[2]) {
  preferences.putInt("minCalib", calibratedRssi[0]);
  preferences.putInt("maxCalib", calibratedRssi[1]);
}

// Read calibration from non-volatile memory
void readCalibrationStorage(int calibratedRssi[2]) {
  calibratedRssi[0] = preferences.getInt("minCalib", DEFAULT_MIN_CALIBRATION);
  calibratedRssi[1] = preferences.getInt("maxCalib", DEFAULT_MAX_CALIBRATION);
}

// Clear everything and start from scratch
void clearReset() {
  preferences.clear();
  esp_restart();
}
