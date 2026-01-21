#include "usb.h"

#ifdef BATTERY_MONITORING
UsbSerial::UsbSerial(Settings *s, RX5808 *r, Battery *b)
  : settings(s), receiver(r), battery(b)
#else
UsbSerial::UsbSerial(Settings *s, RX5808 *r)
  : settings(s), receiver(r)
#endif
{
  serialBufferPos = 0;
  serialBufferOverflow = false;
}

// Start serial connection
void UsbSerial::beginSerial(unsigned long baud) {
  // Don't start connection if already running
  if (serialOn) return;

  Serial.begin(baud);

  serialOn = true;
}

// Flush incoming serial buffer to prevent building up of requests when not active
void UsbSerial::flushIncoming() {
  if (!serialOn) return;

  while (Serial.available()) {
    Serial.read();
  }
}

// Start listening for commands
void UsbSerial::listen() {
  while (Serial.available()) {
    const char c = Serial.read();

    // Ignore carriage return
    if (c == '\r') {
      continue;
    }

    // Accumulate until newline
    if (c != '\n') {
      if (serialBufferPos < SERIAL_BUFFER_LENGTH - 1) {
        serialBuffer[serialBufferPos++] = c;
      } else {
        serialBufferOverflow = true;
      }
      continue;
    }

    // Ignore empty lines
    if (serialBufferPos == 0) {
      resetSerialBuffer();
      continue;
    }

    // Error on buffer overflow
    if (serialBufferOverflow) {
      sendError("", "input JSON too long");
      return;
    }

    // Null-terminate buffer
    serialBuffer[serialBufferPos] = '\0';

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, serialBuffer);

    // Send invalid json error
    if (err || !doc.is<JsonObject>()) {
      sendError("", "invalid JSON");
      return;
    }

    // All event, location and payload keys must be present
    if (doc.size() != 3) {
      sendError("", "all 'event', 'location' and 'payload' keys are required");
      return;
    }

    // Only event, location and payload keys allowed
    for (JsonPair kv : doc.as<JsonObject>()) {
      const char *key = kv.key().c_str();
      if (strcmp(key, "event") != 0 && strcmp(key, "location") != 0 && strcmp(key, "payload") != 0) {
        sendError("", "only 'event', 'location' and 'payload' keys are allowed");
        return;
      }
    }

    // Ensure event is string
    if (!doc["event"].is<const char *>()) {
      sendError("", "'event' must be a string");
      return;
    }

    // Ensure location is string
    if (!doc["location"].is<const char *>()) {
      sendError("", "'location' must be a string");
      return;
    }

    // Ensure only get and post are accepted as event
    if (strcmp(doc["event"], "get") != 0 && strcmp(doc["event"], "post") != 0) {
      sendError("", "'event' must be 'get' or 'post'");
      return;
    }

#ifdef BATTERY_MONITORING
    // Ensure only values, settings, calibration and battery are accepted as location
    if (strcmp(doc["location"], "values") != 0 && strcmp(doc["location"], "settings") != 0
        && strcmp(doc["location"], "calibration") != 0 && strcmp(doc["location"], "battery") != 0) {
      sendError("", "'location' must be 'values', 'settings', 'calibration', or 'battery'");
      return;
    }

    // No post endpoint for battery
    if (strcmp(doc["event"], "post") == 0 && strcmp(doc["location"], "battery") == 0) {
      sendError("", "invalid event 'post' for location 'battery'");
      return;
    }
#else
    // Ensure only values, settings and calibration are accepted as location
    if (strcmp(doc["location"], "values") != 0 && strcmp(doc["location"], "settings") != 0
        && strcmp(doc["location"], "calibration") != 0) {
      sendError("", "'location' must be 'values', 'settings', or 'calibration'");
      return;
    }
#endif

    // Pass off to relevant handler
    if (strcmp(doc["event"], "get") == 0) {
      handleGet(doc);
    } else if (strcmp(doc["event"], "post") == 0) {
      handlePost(doc);
    }

    // Reset for next message
    resetSerialBuffer();
  }
}

// Handler to run relevant endpoint function on GET
void UsbSerial::handleGet(JsonDocument &doc) {
  // Document must have payload as object
  if (!doc["payload"].is<JsonObject>()) {
    sendError(doc["location"], "'payload' must be an object");
    return;
  }

  // Payload must be empty
  if (doc["payload"].size() != 0) {
    sendError(doc["location"], "'payload' object must be empty for 'get' event");
    return;
  }

  // Run correct function based on location
  if (strcmp(doc["location"], "values") == 0) handleGetValues();
  if (strcmp(doc["location"], "settings") == 0) handleGetSettings();
  if (strcmp(doc["location"], "calibration") == 0) handleGetCalibration();
#ifdef BATTERY_MONITORING
  if (strcmp(doc["location"], "battery") == 0) handleGetBattery();
#endif
}

// Handler to run relevant endpoint function on POST
void UsbSerial::handlePost(JsonDocument &doc) {
  // Document must have payload as object
  if (!doc["payload"].is<JsonObject>()) {
    sendError(doc["location"], "'payload' must be an object");
    return;
  }

  // Payload can't be empty
  if (doc["payload"].size() == 0) {
    sendError(doc["location"], "'payload' object must contain at least one key");
    return;
  }

  // Run correct function based on location
  if (strcmp(doc["location"], "values") == 0) handlePostValues(doc);
  if (strcmp(doc["location"], "settings") == 0) handlePostSettings(doc);
  if (strcmp(doc["location"], "calibration") == 0) handlePostCalibration(doc);
}

// Enpoint for getting scanned values
// These values aren't actual rssi values, rather the analog-to-digital converter reading
// Will be within a range of 0 to 4095 inclusive
void UsbSerial::handleGetValues() {
  JsonDocument doc;

  // Set headers
  doc["event"] = "get";
  doc["location"] = "settings";

  // Safely get lowband state
  xSemaphoreTake(receiver->lowbandMutex, portMAX_DELAY);
  bool lowband = receiver->lowband.get();
  xSemaphoreGive(receiver->lowbandMutex);

  // Payload object
  JsonObject payload = doc["payload"].to<JsonObject>();

  // Add frequency information to payload
  int min_freq = lowband ? LOWBAND_MIN_FREQUENCY : HIGHBAND_MIN_FREQUENCY;
  payload["lowband"] = lowband;
  payload["min_frequency"] = min_freq;
  payload["max_frequency"] = min_freq + SCAN_FREQUENCY_RANGE;

  // Calculate number of scanned values based off interval
  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  float interval = settings->scanInterval.get();
  xSemaphoreGive(settings->settingsMutex);
  int numScannedValues = (SCAN_FREQUENCY_RANGE / interval) + 1;  // +1 for final number inclusion

  // Create values array in payload
  JsonArray values = payload["values"].to<JsonArray>();

  // Get receiver values
  for (int i = 0; i < numScannedValues; i++) {
    int rssi;
    xSemaphoreTake(receiver->scanMutex, portMAX_DELAY);
    rssi = receiver->rssiValues.get(i);
    xSemaphoreGive(receiver->scanMutex);

    values.add(rssi);
  }

  sendJson(doc);
}

// Endpoint for setting high or low band
void UsbSerial::handlePostValues(JsonDocument &doc) {
  // Check keys
  if (doc["payload"].size() != 1 || !doc["payload"]["lowband"].is<JsonVariant>()) {
    sendError(doc["location"], "'lowband' must be the only key");
    return;
  }

  // Check key type
  if (!doc["payload"]["lowband"].is<bool>()) {
    sendError(doc["location"], "'lowband' must be a boolean");
    return;
  }

  // Update receiver lowband state
  xSemaphoreTake(receiver->lowbandMutex, portMAX_DELAY);
  receiver->lowband.set(doc["payload"]["lowband"]);
  xSemaphoreGive(receiver->lowbandMutex);

  JsonDocument resp;

  // Set headers
  resp["event"] = "post";
  resp["location"] = "values";
  resp["payload"]["status"] = "ok";

  sendJson(resp);
}

// Endpoint for getting settings indices
// Scan interval settings { 2.5, 5, 10 }
// Buzzer settings { On, Off }
// Battery alarm settings { 3.6, 3.3, 3.0 }
void UsbSerial::handleGetSettings() {
  JsonDocument doc;

  // Set headers
  doc["event"] = "get";
  doc["location"] = "settings";

  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  doc["payload"]["scan_interval_index"] = settings->scanIntervalIndex.get();
  doc["payload"]["scan_interval"] = settings->scanInterval.get();
  doc["payload"]["buzzer_index"] = settings->buzzerIndex.get();
  doc["payload"]["buzzer"] = settings->buzzer.get();
#ifdef BATTERY_MONITORING
  doc["payload"]["battery_alarm_index"] = settings->batteryAlarmIndex.get();
  doc["payload"]["battery_alarm"] = settings->batteryAlarm.get();
#endif
  xSemaphoreGive(settings->settingsMutex);

  sendJson(doc);
}

// Endpoint for updating settings indices
// Scan interval settings { 2.5, 5, 10 }
// Buzzer settings { On, Off }
// Battery alarm settings { 3.6, 3.3, 3.0 }
void UsbSerial::handlePostSettings(JsonDocument &doc) {
#ifdef BATTERY_MONITORING
  // Only scan_interval_index, buzzer_index and battery_alarm_index keys allowed
  for (JsonPair kv : doc["payload"].as<JsonObject>()) {
    const char *key = kv.key().c_str();
    if (strcmp(key, "scan_interval_index") != 0 && strcmp(key, "buzzer_index") != 0 && strcmp(key, "battery_alarm_index") != 0) {
      sendError(doc["location"], "only 'scan_interval_index', 'buzzer_index' and 'battery_alarm_index' keys are allowed");
      return;
    }
  }
#else
  // Only scan_interval_index and buzzer_index keys allowed
  for (JsonPair kv : doc["payload"].as<JsonObject>()) {
    const char *key = kv.key().c_str();
    if (strcmp(key, "scan_interval_index") != 0 && strcmp(key, "buzzer_index") != 0) {
      sendError(doc["location"], "only 'scan_interval_index' and 'buzzer_index' keys are allowed");
      return;
    }
  }
#endif

  // Validate type and value of scan_interval_index
  if (doc["payload"]["scan_interval_index"].is<JsonVariant>()) {
    if (!doc["payload"]["scan_interval_index"].is<int>()) {
      sendError(doc["location"], "'scan_interval_index' must be an integer");
      return;
    }
    if (doc["payload"]["scan_interval_index"] < 0 || doc["payload"]["scan_interval_index"] > 2) {
      sendError(doc["location"], "'scan_interval_index' must be between 0 and 2 inclusive");
      return;
    }
  }

  // Validate type and value of buzzer_index
  if (doc["payload"]["buzzer_index"].is<JsonVariant>()) {
    if (!doc["payload"]["buzzer_index"].is<int>()) {
      sendError(doc["location"], "'buzzer_index' must be an integer");
      return;
    }
    if (doc["payload"]["buzzer_index"] < 0 || doc["payload"]["buzzer_index"] > 1) {
      sendError(doc["location"], "'buzzer_index' must be 0 or 1");
      return;
    }
  }

#ifdef BATTERY_MONITORING
  // Validate type and value of battery_alarm_index
  if (doc["payload"]["battery_alarm_index"].is<JsonVariant>()) {
    if (!doc["payload"]["battery_alarm_index"].is<int>()) {
      sendError(doc["location"], "'battery_alarm_index' must be an integer");
      return;
    }
    if (doc["payload"]["battery_alarm_index"] < 0 || doc["payload"]["battery_alarm_index"] > 2) {
      sendError(doc["location"], "'battery_alarm_index' must be between 0 and 2 inclusive");
      return;
    }
  }
#endif

  // Apply valid updates
  if (doc["payload"]["scan_interval_index"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->scanIntervalIndex.set(doc["payload"]["scan_interval_index"]);
    xSemaphoreGive(settings->settingsMutex);

    // Need to restart scanning for interval update to work
    receiver->stopScan();
    receiver->startScan();
  }
  if (doc["payload"]["buzzer_index"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->buzzerIndex.set(doc["payload"]["buzzer_index"]);
    xSemaphoreGive(settings->settingsMutex);
  }
#ifdef BATTERY_MONITORING
  if (doc["payload"]["battery_alarm_index"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->batteryAlarmIndex.set(doc["payload"]["battery_alarm_index"]);
    xSemaphoreGive(settings->settingsMutex);
  }
#endif

  JsonDocument resp;

  // Set headers
  resp["event"] = "post";
  resp["location"] = "values";
  resp["payload"]["status"] = "ok";

  sendJson(resp);
}

// Endpoint for getting current calibration values
// Returns in the form of { low_value, high_value }
// These values aren't actual rssi values, rather the analog-to-digital converter reading
// Will be within a range of 0 to 4095 inclusive
void UsbSerial::handleGetCalibration() {
  JsonDocument doc;

  // Set headers
  doc["event"] = "get";
  doc["location"] = "calibration";

  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  doc["payload"]["low_rssi"] = settings->lowCalibratedRssi.get();
  doc["payload"]["high_rssi"] = settings->highCalibratedRssi.get();
  xSemaphoreGive(settings->settingsMutex);

  sendJson(doc);
}

// Endpoint for setting high and low calibration values
// Must be within a range of 0 to 4095 inclusive, with low value less than high value
void UsbSerial::handlePostCalibration(JsonDocument &doc) {
  // Only high_rssi and low_rssi keys allowed
  for (JsonPair kv : doc["payload"].as<JsonObject>()) {
    const char *key = kv.key().c_str();
    if (strcmp(key, "high_rssi") != 0 && strcmp(key, "low_rssi") != 0) {
      sendError(doc["location"], "only 'high_rssi' and 'low_rssi' keys are allowed");
      return;
    }
  }

  // Safely get calibrated rssi values
  xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
  int newHigh = settings->highCalibratedRssi.get();
  int newLow = settings->lowCalibratedRssi.get();
  xSemaphoreGive(settings->settingsMutex);

  // Validate type and value of high_rssi
  if (doc["payload"]["high_rssi"].is<JsonVariant>()) {
    if (!doc["payload"]["high_rssi"].is<int>()) {
      sendError(doc["location"], "'high_rssi' must be an integer");
      return;
    }
    newHigh = doc["payload"]["high_rssi"];
    if (newHigh < 0 || newHigh > 4095) {
      sendError(doc["location"], "'high_rssi' must be between 0 and 4095 inclusive");
      return;
    }
  }

  // Validate type and value of low_rssi
  if (doc["payload"]["low_rssi"].is<JsonVariant>()) {
    if (!doc["payload"]["low_rssi"].is<int>()) {
      sendError(doc["location"], "'low_rssi' must be an integer");
      return;
    }
    newLow = doc["payload"]["low_rssi"];
    if (newLow < 0 || newLow > 4095) {
      sendError(doc["location"], "'low_rssi' must be between 0 and 4095 inclusive");
      return;
    }
  }

  // high_rssi must be greater than low_rssi
  if (newHigh <= newLow) {
    sendError(doc["location"], "'high_rssi' must be greater than 'low_rssi' (considering new or existing values)");
    return;
  }

  // Apply valid updates
  if (doc["payload"]["high_rssi"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->highCalibratedRssi.set(newHigh);
    xSemaphoreGive(settings->settingsMutex);
  }
  if (doc["payload"]["low_rssi"].is<JsonVariant>()) {
    xSemaphoreTake(settings->settingsMutex, portMAX_DELAY);
    settings->lowCalibratedRssi.set(newLow);
    xSemaphoreGive(settings->settingsMutex);
  }

  JsonDocument resp;

  // Set headers
  resp["event"] = "post";
  resp["location"] = "values";
  resp["payload"]["status"] = "ok";

  sendJson(resp);
}

#ifdef BATTERY_MONITORING
// Endpoint for getting battery voltage
void UsbSerial::handleGetBattery() {
  JsonDocument doc;

  // Set headers
  doc["event"] = "get";
  doc["location"] = "battery";

  xSemaphoreTake(battery->batteryMutex, portMAX_DELAY);
  doc["payload"]["voltage"] = battery->currentVoltage.get();
  xSemaphoreGive(battery->batteryMutex);

  sendJson(doc);
}
#endif

// Send json to serial
void UsbSerial::sendJson(JsonDocument &doc) {
  serializeJson(doc, Serial);
  Serial.println();
}

// Send json error message
void UsbSerial::sendError(const char *location, const char *msg) {
  JsonDocument doc;

  doc["event"] = "error";
  doc["location"] = location;
  doc["payload"]["status"] = msg;

  sendJson(doc);
  resetSerialBuffer();
}

// Reset serial buffer state
void UsbSerial::resetSerialBuffer() {
  serialBufferPos = 0;
  serialBufferOverflow = false;
}
