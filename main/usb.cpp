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
      sendError("input JSON too long");
      resetSerialBuffer();
      return;
    }

    // Null-terminate buffer
    serialBuffer[serialBufferPos] = '\0';

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, serialBuffer);

    // Send invalid json error
    if (err || !doc.is<JsonObject>()) {
      sendError("invalid JSON");
      resetSerialBuffer();
      return;
    }

    // All event, location and payload keys must be present
    if (doc.size() != 3) {
      sendError("all 'event', 'location' and 'payload' keys are required");
      resetSerialBuffer();
      return;
    }

    // Only event, location and payload keys allowed
    for (JsonPair kv : doc.as<JsonObject>()) {
      const char *key = kv.key().c_str();
      if (strcmp(key, "event") != 0 && strcmp(key, "location") != 0 && strcmp(key, "payload") != 0) {
        sendError("only 'event', 'location' and 'payload' keys are allowed");
        resetSerialBuffer();
        return;
      }
    }

    // Ensure event is string
    if (!doc["event"].is<const char *>()) {
      sendError("'event' must be a string");
      resetSerialBuffer();
      return;
    }

    // Ensure location is string
    if (!doc["location"].is<const char *>()) {
      sendError("'location' must be a string");
      resetSerialBuffer();
      return;
    }

    // Ensure only get and post are accepted as event
    if (strcmp(doc["event"], "get") != 0 && strcmp(doc["event"], "post") != 0) {
      sendError("'event' must be 'get' or 'post'");
      resetSerialBuffer();
      return;
    }

#ifdef BATTERY_MONITORING
    // Ensure only values, settings, calibration and battery are accepted as location
    if (strcmp(doc["location"], "values") != 0 && strcmp(doc["location"], "settings") != 0
        && strcmp(doc["location"], "calibration") != 0 && strcmp(doc["location"], "battery") != 0) {
      sendError("'location' must be 'values', 'settings', 'calibration', or 'battery'");
      resetSerialBuffer();
      return;
    }

    // No post endpoint for battery
    if (strcmp(doc["event"], "post") == 0 && strcmp(doc["location"], "battery") == 0) {
      sendError("invalid event 'post' for location 'battery'");
      resetSerialBuffer();
      return;
    }
#else
    // Ensure only values, settings and calibration are accepted as location
    if (strcmp(doc["location"], "values") != 0 && strcmp(doc["location"], "settings") != 0
        && strcmp(doc["location"], "calibration") != 0) {
      sendError("'location' must be 'values', 'settings', or 'calibration'");
      resetSerialBuffer();
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
  Serial.println("post");
}

// Enpoint for getting scanned values
// These values aren't actual rssi values, rather the analog-to-digital converter reading
// Will be within a range of 0 to 4095 inclusive
void UsbSerial::handleGetValues() {}

// Endpoint for setting high or low band
void UsbSerial::handlePostValues() {}

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
void UsbSerial::handlePostSettings() {}

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
void UsbSerial::handlePostCalibration() {}

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
void UsbSerial::sendError(const char *msg) {
  JsonDocument doc;

  doc["event"] = "error";
  doc["location"] = "";
  doc["payload"] = msg;

  sendJson(doc);
}

// Reset serial buffer state
void UsbSerial::resetSerialBuffer() {
  serialBufferPos = 0;
  serialBufferOverflow = false;
}
