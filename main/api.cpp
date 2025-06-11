#include "api.h"

Api::Api(Settings *s, RX5808 *r, Battery *b)
  : wifiOn(false),
    settings(s), module(r), battery(b),
    server(80) {

  server.onNotFound([this](AsyncWebServerRequest *request) {
    handleNotFound(request);
  });

  server.on("/api/values", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetValues(request);
  });

  server.on(
    "/api/values", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handlePostValues(request, data, len, index, total);
    });

  server.on("/api/battery", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetBattery(request);
  });

  server.on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetSettings(request);
  });

  server.on("/api/calibration", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetCalibration(request);
  });
}

// Start wifi hotspot
void Api::startWifi() {
  // Do nothing if wifi already on
  if (wifiOn) return;

  // Set static ip
  IPAddress ip, gateway, subnet;
  ip.fromString(WIFI_IP);
  gateway.fromString(WIFI_GATEWAY);
  subnet.fromString(WIFI_SUBNET);
  WiFi.softAPConfig(ip, gateway, subnet);

  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
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

// Return 404 not found error
void Api::handleNotFound(AsyncWebServerRequest *request) {
  JsonDocument doc;

  doc["error"] = "404: Not found";
  doc["path"] = request->url();

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->setCode(404);

  serializeJson(doc, *response);
  request->send(response);
}

// Enpoint for getting scanned values
// These values aren't actual rssi values, rather the analog-to-digital converter reading
// Will be within a range of 0 to 4095 inclusive
void Api::handleGetValues(AsyncWebServerRequest *request) {
  JsonDocument doc;

  // Add frequency information to json
  int min_freq = module->lowband.get() ? LOWBAND_MIN_FREQUENCY : HIGHBAND_MIN_FREQUENCY;
  doc["lowband"] = module->lowband.get();
  doc["min_frequency"] = min_freq;
  doc["max_frequency"] = min_freq + SCAN_FREQUENCY_RANGE;

  // Calculate number of scanned values based off of interval
  int interval = settings->scanInterval.get();
  int numScannedValues = (SCAN_FREQUENCY_RANGE / interval) + 1;  // +1 for final number inclusion

  JsonArray values = doc["values"].to<JsonArray>();

  for (int i = 0; i < numScannedValues; i++) {
    // Safely get current rssi
    int rssi;
    if (xSemaphoreTake(module->scanMutex, portMAX_DELAY)) {
      rssi = module->rssiValues.get(i);

      xSemaphoreGive(module->scanMutex);
    }

    values.add(rssi);
  }

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}

// Endpoint for setting high or low band
void Api::handlePostValues(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;

  // Deserialise and validatate json
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  // Check keys
  if (doc.size() != 1 || !doc.containsKey("lowband")) {
    request->send(400, "application/json", "{\"error\":\"JSON must contain exactly one key: 'lowband'\"}");
    return;
  }

  // Check key type
  if (!doc["lowband"].is<bool>()) {
    request->send(400, "application/json", "{\"error\":\"'lowband' must be a boolean\"}");
    return;
  }

  // Update module lowband state
  module->lowband.set(doc["lowband"]);

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

// Endpoint for getting battery voltage
void Api::handleGetBattery(AsyncWebServerRequest *request) {
  JsonDocument doc;

  doc["voltage"] = battery->currentVoltage.get();

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}

// Endpoint for getting settings indices
// Scan interval settings { 5, 10, 20 }
// Buzzer settings { On, Off }
// Battery alarm settings { 3.6, 3.3, 3.0 }
void Api::handleGetSettings(AsyncWebServerRequest *request) {
  JsonDocument doc;

  doc["scan_interval_index"] = settings->scanIntervalIndex.get();
  doc["scan_interval"] = settings->scanInterval.get();
  doc["buzzer_index"] = settings->buzzerIndex.get();
  doc["buzzer"] = settings->buzzer.get();
  doc["battery_alarm_index"] = settings->batteryAlarmIndex.get();
  doc["battery_alarm"] = settings->batteryAlarm.get();

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}

// Endpoint for getting current calibration values
// Returns in the form of { low_value, high_value }
// These values aren't actual rssi values, rather the analog-to-digital converter reading
// Will be within a range of 0 to 4095 inclusive
void Api::handleGetCalibration(AsyncWebServerRequest *request) {
  JsonDocument doc;

  doc["low"] = settings->lowCalibratedRssi.get();
  doc["high"] = settings->highCalibratedRssi.get();

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}
