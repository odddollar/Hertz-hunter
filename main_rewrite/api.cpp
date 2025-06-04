#include "api.h"

Api::Api(Settings *s, RX5808 *r)
  : wifiOn(false),
    settings(s), module(r),
    server(80) {
}

// Start wifi hotspot
void Api::startWifi() {
  // Do nothing if wifi already on
  if (wifiOn) return;

  WiFi.softAP(SSID, PASSWORD);
  server.begin();

  wifiOn = true;
}

// Stop wifi hotspot
void Api::stopWifi() {
  // Do nothing if wifi already off
  if (!wifiOn) return;

  server.end();
  WiFi.softAPdisconnect(true);

  wifiOn = false;
}
