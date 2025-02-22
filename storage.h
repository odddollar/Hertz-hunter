#ifndef STORAGE_H
#define STORAGE_H

#include <Preferences.h>

// External declaration for settings storage
extern Preferences preferences;

// Write and read settings data to storage
void writeSettingsStorage(int settings[3]);
void readSettingsStorage(int settings[3]);

#endif
