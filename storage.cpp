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
    settings[i] = preferences.getInt(String(i).c_str(), 0);
  }
}
