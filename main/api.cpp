#include "api.h"

// Initialise api
API::API(const char *s, const char *pwd, int (*rssiV)[61], int *nSV, int *bV, int (*sI)[3], int (*cRSSI)[2], SemaphoreHandle_t m)
  : wifiOn(false), ssid(s), password(pwd), rssiValues(rssiV), numScannedValues(nSV), batteryVoltage(bV),
    settingsIndices(sI), calibratedRssi(cRSSI), mutex(m), server(80) {

  // 404 endpoint
  server.onNotFound([](AsyncWebServerRequest *request) {
    JsonDocument doc;

    doc["error"] = "404: Not found";
    doc["path"] = request->url();

    String json;
    serializeJson(doc, json);

    request->send(404, "application/json", json);
  });

  // Enpoint for getting scanned values
  // These values aren't actual rssi values, rather the analog-to-digital converter reading
  // Will be within a range of 0 to 4095 inclusive
  server.on("/api/values", HTTP_GET, [this](AsyncWebServerRequest *request) {
    JsonDocument doc;

    // Create copy of rssi values to return
    int rssiValuesCopy[61];
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      memcpy(rssiValuesCopy, *rssiValues, sizeof(int) * (*numScannedValues));
      xSemaphoreGive(mutex);
    }

    // Add each value to json array
    JsonArray values = doc["values"].to<JsonArray>();
    for (int i = 0; i < *numScannedValues; i++) {
      values.add(rssiValuesCopy[i]);
    }

    String json;
    serializeJson(doc, json);

    request->send(200, "application/json", json);
  });

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

  // Endpoint for getting current calibration values
  // Returns in the form of { low_value, high_value }
  // These values aren't actual rssi values, rather the analog-to-digital converter reading
  // Will be within a range of 0 to 4095 inclusive
  server.on("/api/calibration", HTTP_GET, [this](AsyncWebServerRequest *request) {
    JsonDocument doc;

    doc["low"] = (*calibratedRssi)[0];
    doc["high"] = (*calibratedRssi)[1];

    String json;
    serializeJson(doc, json);

    request->send(200, "application/json", json);
  });
}

// Start wifi hotspot
void API::startWifi() {
  // Do nothing if wifi already on
  if (wifiOn) { return; }

  WiFi.softAP(ssid, password);
  server.begin();

  wifiOn = true;
}

// Stop wifi hotspot
void API::stopWifi() {
  // Do nothing if wifi already off
  if (!wifiOn) { return; }

  server.end();
  WiFi.softAPdisconnect(true);

  wifiOn = false;
}

// Get IP address
const char *API::getIP() {
  static char ipStr[16];
  IPAddress ip = WiFi.softAPIP();
  snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  return ipStr;
}
