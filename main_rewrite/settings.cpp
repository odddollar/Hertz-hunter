#include "settings.h"

Settings::Settings()
  // Initialise to defaults
  : scanIntervalIndex(DEFAULT_INDEX), scanInterval(DEFAULT_SCAN_INTERVAL),
    buzzerIndex(DEFAULT_INDEX), buzzer(DEFAULT_BUZZER),
    batteryAlarmIndex(DEFAULT_INDEX), batteryAlarm(DEFAULT_BATTERY_ALARM),
    lowCalibratedRssi(DEFAULT_LOW_CALIBRATED_RSSI), highCalibratedRssi(DEFAULT_HIGH_CALIBRATED_RSSI),
    initialReadDone(false) {

  // Set preferences namespace
  preferences.begin("settings", false);

  // When interval index changes, update actual interval
  scanIntervalIndex.onChange([this](int val) {
    scanInterval.set(5 * pow(2, val));
    if (initialReadDone) saveSettingsStorage("scan_interval_index", val);
  });

  // When buzzer index changes, update buzzer state
  buzzerIndex.onChange([this](int val) {
    buzzer.set(val == 0 ? true : false);
    if (initialReadDone) saveSettingsStorage("buzzer_index", val);
  });

  // When battery index changes, update alarm threshold
  batteryAlarmIndex.onChange([this](int val) {
    batteryAlarm.set(-3 * val + 36);
    if (initialReadDone) saveSettingsStorage("battery_alarm_index", val);
  });

  // Write calibration to storage on change
  lowCalibratedRssi.onChange([this](int val) {
    if (initialReadDone) saveSettingsStorage("low_calibrated_rssi", val);
  });

  // Write calibration to storage on change
  highCalibratedRssi.onChange([this](int val) {
    if (initialReadDone) saveSettingsStorage("high_calibrated_rssi", val);
  });
}

// Save given value to given key
void Settings::saveSettingsStorage(const char *key, int value) {
  preferences.putInt(key, value);
  Serial.printf("%s: %i\n", key, value);
}


// Load all settings from memory
void Settings::loadSettingsStorage() {
  scanIntervalIndex.set(preferences.getInt("scan_interval_index", DEFAULT_INDEX));
  buzzerIndex.set(preferences.getInt("buzzer_index", DEFAULT_INDEX));
  batteryAlarmIndex.set(preferences.getInt("battery_alarm_index", DEFAULT_INDEX));
  lowCalibratedRssi.set(preferences.getInt("low_calibrated_rssi", DEFAULT_LOW_CALIBRATED_RSSI));
  highCalibratedRssi.set(preferences.getInt("high_calibrated_rssi", DEFAULT_HIGH_CALIBRATED_RSSI));

  // Used to prevent reading from non-volatile memory, updating variables, then immediately writing same value
  // Prevents unnecessary flash wear
  initialReadDone = true;
}

// Clear everything and reset
void Settings::clearReset() {
  preferences.clear();
  esp_restart();
}
