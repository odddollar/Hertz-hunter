#include "api.h"

Api::Api(Settings *s)
  : wifiOn(false), settings(s) {
}

// Start wifi hotspot
void Api::startWifi() {
  // Do nothing if wifi already on
  if (wifiOn) return;

  WiFi.softAP(SSID, PASSWORD);

  wifiOn = true;
}

// Stop wifi hotspot
void Api::stopWifi() {
  // Do nothing if wifi already off
  if (!wifiOn) return;

  WiFi.softAPdisconnect(true);

  wifiOn = false;
}
