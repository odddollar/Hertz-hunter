#include "api.h"

// Initialise api
API::API(const char *s, const char *pwd, int *bV)
  : server(80) {
  wifiOn = false;
  ssid = s;
  password = pwd;
  batteryVoltage = bV;

  // Get battery voltage
  server.on("/api/battery", HTTP_GET, [this](AsyncWebServerRequest *request) {
    // Format voltage reading
    char formattedVoltage[16];
    snprintf(formattedVoltage, sizeof(formattedVoltage), "{\"voltage\":%d.%d}", (*batteryVoltage) / 10, (*batteryVoltage) % 10);
    request->send(200, "application/json", formattedVoltage);
  });
}

// Start wifi hotspot
void API::startWifi() {
  // Do nothing if wifi already on
  if (wifiOn) { return; }

  Serial.println("Starting Access Point...");
  WiFi.softAP(ssid, password);
  server.begin();  // Start the web server
  Serial.println("Access Point Started.");

  wifiOn = true;
}

// Stop wifi hotspot
void API::stopWifi() {
  // Do nothing if wifi already off
  if (!wifiOn) { return; }

  Serial.println("Stopping Access Point...");
  server.end();
  WiFi.softAPdisconnect(true);
  Serial.println("Access Point Stopped.");

  wifiOn = false;
}

// Get IP address
const char *API::getIP() {
  static char ipStr[16];
  IPAddress ip = WiFi.softAPIP();
  snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  return ipStr;
}
