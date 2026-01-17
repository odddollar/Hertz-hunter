#include "api.h"

#ifdef BATTERY_MONITORING
Api::Api(Settings *s, RX5808 *r, Battery *b)
  : wifiOn(false), settings(s), receiver(r), battery(b),
    server(80)
#else
Api::Api(Settings *s, RX5808 *r)
  : wifiOn(false), settings(s), receiver(r),
    server(80)
#endif
{
  server.onNotFound([this](AsyncWebServerRequest *request) {
    handleNotFound(request);
  });

  server.on("/api/values", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetValues(request);
  });

  server.on(
    "/api/values", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handlePostValues(request, data, len, index, total);
    });

  server.on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetSettings(request);
  });

  server.on(
    "/api/settings", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handlePostSettings(request, data, len, index, total);
    });

  server.on("/api/calibration", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetCalibration(request);
  });

  server.on(
    "/api/calibration", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handlePostCalibration(request, data, len, index, total);
    });

#ifdef BATTERY_MONITORING
  server.on("/api/battery", HTTP_GET, [this](AsyncWebServerRequest *request) {
    handleGetBattery(request);
  });
#endif
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

  // Safely get lowband state
  xSemaphoreTake(receiver->lowbandMutex, portMAX_DELAY);
  bool lowband = receiver->lowband.get();
  xSemaphoreGive(receiver->lowbandMutex);

  // Add frequency information to json
  int min_freq = lowband ? LOWBAND_MIN_FREQUENCY : HIGHBAND_MIN_FREQUENCY;
  doc["lowband"] = lowband;
  doc["min_frequency"] = min_freq;
  doc["max_frequency"] = min_freq + SCAN_FREQUENCY_RANGE;

  // Calculate number of scanned values based off of interval
  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  float interval = settings->scanInterval.get();
  xSemaphoreGive(settings->settingsMutex);
  int numScannedValues = (SCAN_FREQUENCY_RANGE / interval) + 1;  // +1 for final number inclusion

  JsonArray values = doc["values"].to<JsonArray>();

  for (int i = 0; i < numScannedValues; i++) {
    // Safely get current rssi
    int rssi;
    xSemaphoreTake(receiver->scanMutex, portMAX_DELAY);
    rssi = receiver->rssiValues.get(i);
    xSemaphoreGive(receiver->scanMutex);

    values.add(rssi);
  }

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}

// Endpoint for setting high or low band
void Api::handlePostValues(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;

  // Deserialise and validate json
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
    return;
  }

  // Check keys
  if (doc.size() != 1 || !doc["lowband"].is<JsonVariant>()) {
    request->send(400, "application/json", "{\"error\":\"'lowband' must be the only key\"}");
    return;
  }

  // Check key type
  if (!doc["lowband"].is<bool>()) {
    request->send(400, "application/json", "{\"error\":\"'lowband' must be a boolean\"}");
    return;
  }

  // Update receiver lowband state
  xSemaphoreTake(receiver->lowbandMutex, portMAX_DELAY);
  receiver->lowband.set(doc["lowband"]);
  xSemaphoreGive(receiver->lowbandMutex);

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

// Endpoint for getting settings indices
// Scan interval settings { 2.5, 5, 10 }
// Buzzer settings { On, Off }
// Battery alarm settings { 3.6, 3.3, 3.0 }
void Api::handleGetSettings(AsyncWebServerRequest *request) {
  JsonDocument doc;

  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  doc["scan_interval_index"] = settings->scanIntervalIndex.get();
  doc["scan_interval"] = settings->scanInterval.get();
  doc["buzzer_index"] = settings->buzzerIndex.get();
  doc["buzzer"] = settings->buzzer.get();
#ifdef BATTERY_MONITORING
  doc["battery_alarm_index"] = settings->batteryAlarmIndex.get();
  doc["battery_alarm"] = settings->batteryAlarm.get();
#endif
  xSemaphoreGive(settings->settingsMutex);

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}

// Endpoint for updating settings indices
// Scan interval settings { 2.5, 5, 10 }
// Buzzer settings { On, Off }
// Battery alarm settings { 3.6, 3.3, 3.0 }
void Api::handlePostSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;

  // Deserialise and validate json
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
    return;
  }

#ifdef BATTERY_MONITORING
  // Only scan_interval_index, buzzer_index and battery_alarm_index keys allowed
  for (JsonPair kv : doc.as<JsonObject>()) {
    const char *key = kv.key().c_str();
    if (strcmp(key, "scan_interval_index") != 0 && strcmp(key, "buzzer_index") != 0 && strcmp(key, "battery_alarm_index") != 0) {
      request->send(400, "application/json", "{\"error\":\"only 'scan_interval_index', 'buzzer_index' and 'battery_alarm_index' keys are allowed\"}");
      return;
    }
  }
#else
  // Only scan_interval_index and buzzer_index keys allowed
  for (JsonPair kv : doc.as<JsonObject>()) {
    const char *key = kv.key().c_str();
    if (strcmp(key, "scan_interval_index") != 0 && strcmp(key, "buzzer_index") != 0) {
      request->send(400, "application/json", "{\"error\":\"only 'scan_interval_index' and 'buzzer_index' keys are allowed\"}");
      return;
    }
  }
#endif

  // Validate type and value of scan_interval_index
  if (doc["scan_interval_index"].is<JsonVariant>()) {
    if (!doc["scan_interval_index"].is<int>()) {
      request->send(400, "application/json", "{\"error\":\"'scan_interval_index' must be an integer\"}");
      return;
    }
    if (doc["scan_interval_index"] < 0 || doc["scan_interval_index"] > 2) {
      request->send(400, "application/json", "{\"error\":\"'scan_interval_index' must be between 0 and 2 inclusive\"}");
      return;
    }
  }

  // Validate type and value of buzzer_index
  if (doc["buzzer_index"].is<JsonVariant>()) {
    if (!doc["buzzer_index"].is<int>()) {
      request->send(400, "application/json", "{\"error\":\"'buzzer_index' must be an integer\"}");
      return;
    }
    if (doc["buzzer_index"] < 0 || doc["buzzer_index"] > 1) {
      request->send(400, "application/json", "{\"error\":\"'buzzer_index' must be 0 or 1\"}");
      return;
    }
  }

#ifdef BATTERY_MONITORING
  // Validate type and value of battery_alarm_index
  if (doc["battery_alarm_index"].is<JsonVariant>()) {
    if (!doc["battery_alarm_index"].is<int>()) {
      request->send(400, "application/json", "{\"error\":\"'battery_alarm_index' must be an integer\"}");
      return;
    }
    if (doc["battery_alarm_index"] < 0 || doc["battery_alarm_index"] > 2) {
      request->send(400, "application/json", "{\"error\":\"'battery_alarm_index' must be between 0 and 2 inclusive\"}");
      return;
    }
  }
#endif

  // Apply valid updates
  if (doc["scan_interval_index"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->scanIntervalIndex.set(doc["scan_interval_index"]);
    xSemaphoreGive(settings->settingsMutex);

    // Need to restart scanning for interval update to work
    receiver->stopScan();
    receiver->startScan();
  }
  if (doc["buzzer_index"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->buzzerIndex.set(doc["buzzer_index"]);
    xSemaphoreGive(settings->settingsMutex);
  }
#ifdef BATTERY_MONITORING
  if (doc["battery_alarm_index"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->batteryAlarmIndex.set(doc["battery_alarm_index"]);
    xSemaphoreGive(settings->settingsMutex);
  }
#endif

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

// Endpoint for getting current calibration values
// Returns in the form of { low_value, high_value }
// These values aren't actual rssi values, rather the analog-to-digital converter reading
// Will be within a range of 0 to 4095 inclusive
void Api::handleGetCalibration(AsyncWebServerRequest *request) {
  JsonDocument doc;

  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  doc["low_rssi"] = settings->lowCalibratedRssi.get();
  doc["high_rssi"] = settings->highCalibratedRssi.get();
  xSemaphoreGive(settings->settingsMutex);

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}

// Endpoint for setting high and low calibration values
// Must be within a range of 0 to 4095 inclusive, with low value less than high value
void Api::handlePostCalibration(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;

  // Deserialise and validate json
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
    return;
  }

  // Only high_rssi and low_rssi keys allowed
  for (JsonPair kv : doc.as<JsonObject>()) {
    const char *key = kv.key().c_str();
    if (strcmp(key, "high_rssi") != 0 && strcmp(key, "low_rssi") != 0) {
      request->send(400, "application/json", "{\"error\":\"only 'high_rssi' and 'low_rssi' keys are allowed\"}");
      return;
    }
  }

  // Safely get calibrated rssi values
  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  int newHigh = settings->highCalibratedRssi.get();
  int newLow = settings->lowCalibratedRssi.get();
  xSemaphoreGive(settings->settingsMutex);

  // Validate type and value of high_rssi
  if (doc["high_rssi"].is<JsonVariant>()) {
    if (!doc["high_rssi"].is<int>()) {
      request->send(400, "application/json", "{\"error\":\"'high_rssi' must be an integer\"}");
      return;
    }
    newHigh = doc["high_rssi"];
    if (newHigh < 0 || newHigh > 4095) {
      request->send(400, "application/json", "{\"error\":\"'high_rssi' must be between 0 and 4095 inclusive\"}");
      return;
    }
  }

  // Validate type and value of low_rssi
  if (doc["low_rssi"].is<JsonVariant>()) {
    if (!doc["low_rssi"].is<int>()) {
      request->send(400, "application/json", "{\"error\":\"'low_rssi' must be an integer\"}");
      return;
    }
    newLow = doc["low_rssi"];
    if (newLow < 0 || newLow > 4095) {
      request->send(400, "application/json", "{\"error\":\"'low_rssi' must be between 0 and 4095 inclusive\"}");
      return;
    }
  }

  // high_rssi must be greater than low_rssi
  if (newHigh <= newLow) {
    request->send(400, "application/json", "{\"error\":\"'high_rssi' must be greater than 'low_rssi' (considering new or existing values)\"}");
    return;
  }

  // Apply valid updates
  if (doc["high_rssi"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->highCalibratedRssi.set(newHigh);
    xSemaphoreGive(settings->settingsMutex);
  }
  if (doc["low_rssi"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->lowCalibratedRssi.set(newLow);
    xSemaphoreGive(settings->settingsMutex);
  }

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

#ifdef BATTERY_MONITORING
// Endpoint for getting battery voltage
void Api::handleGetBattery(AsyncWebServerRequest *request) {
  JsonDocument doc;

  xSemaphoreTake(battery->batteryMutex, portMAX_DELAY);
  doc["voltage"] = battery->currentVoltage.get();
  xSemaphoreGive(battery->batteryMutex);

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  serializeJson(doc, *response);
  request->send(response);
}
#endif
