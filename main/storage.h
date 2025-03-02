#ifndef STORAGE_H
#define STORAGE_H

#include <Preferences.h>

// Defaults in case nothing stored in memory
#define DEFAULT_SETTING_INDEX 0
#define DEFAULT_MIN_CALIBRATION 0
#define DEFAULT_MAX_CALIBRATION 4096

// External declaration for settings storage
extern Preferences preferences;

// Write and read settings data to storage
void writeSettingsStorage(int settings[3]);
void readSettingsStorage(int settings[3]);

// Write and read calibration data to storage
void writeCalibrationStorage(int calibratedRssi[2]);
void readCalibrationStorage(int calibratedRssi[2]);

#endif
