#include "settings.h"

Settings::Settings()
  // Initialise to defaults
  : scanIntervalIndex(DEFAULT_INDEX), scanInterval(DEFAULT_SCAN_INTERVAL),
    buzzerIndex(DEFAULT_INDEX), buzzer(DEFAULT_BUZZER),
    batteryAlarmIndex(DEFAULT_INDEX), batteryAlarm(DEFAULT_BATTERY_ALARM),
    lowCalibratedRssi(DEFAULT_LOW_CALIBRATED_RSSI), highCalibratedRssi(DEFAULT_HIGH_CALIBRAYED_RSSI) {

  // When interval index changes, update actual interval
  scanIntervalIndex.onChange([this](int val) {
    scanInterval.set(5 * pow(2, val));
  });

  // When buzzer index changes, update buzzer state
  buzzerIndex.onChange([this](int val) {
    buzzer.set(val == 0 ? true : false);
  });

  // When battery index changes, update alarm threshold
  batteryAlarmIndex.onChange([this](int val) {
    batteryAlarm.set(-3 * val + 36);
  });
}
