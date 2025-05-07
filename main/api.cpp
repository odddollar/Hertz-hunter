#include "api.h"

// Initialise api
API::API(const char *s, const char *pwd) {
  wifiOn = false;
  ssid = s;
  password = pwd;
}

// Start wifi hotspot
void API::startWifi() {
  // Do nothing if wifi already on
  if (wifiOn) { return; }

  Serial.println("Starting Access Point...");
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started.");

  wifiOn = true;
}

// Stop wifi hotspot
void API::stopWifi() {
  // Do nothing if wifi already off
  if (!wifiOn) { return; }

  Serial.println("Stopping Access Point...");
  WiFi.softAPdisconnect(true);
  Serial.println("Access Point Stopped.");

  wifiOn = false;
}

// Get IP address
IPAddress API::getIP() {
  return WiFi.softAPIP();
}
