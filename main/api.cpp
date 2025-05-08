#include "api.h"

// Initialise api
API::API(const char *s, const char *pwd, int *bV, int (*sI)[3])
  : wifiOn(false), ssid(s), password(pwd), batteryVoltage(bV), settingsIndices(sI), server(80) {

  // Endpoint for getting battery voltage
  server.on("/api/battery", HTTP_GET, [this](AsyncWebServerRequest *request) {
    JsonDocument doc;

    // Convert int to float with one decimal place
    doc["voltage"] = static_cast<float>(*batteryVoltage) / 10.0;

    String json;
    serializeJson(doc, json);

    request->send(200, "application/json", json);
  });

  // Endpoint for getting settings indices
  // This doesn't return the actual settings value, just the index
  // Scan interval settings { 5, 10, 20 }
  // Buzzer settings { On, Off }
  // Battery alarm settings { 3.6, 3.3, 3.0 }
  server.on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
    JsonDocument doc;

    doc["scan_interval"] = (*settingsIndices)[0];
    doc["buzzer"] = (*settingsIndices)[1];
    doc["battery_alarm"] = (*settingsIndices)[2];

    String json;
    serializeJson(doc, json);

    request->send(200, "application/json", json);
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
