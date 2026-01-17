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
      sendError("Input JSON too long");
      resetSerialBuffer();
      return;
    }

    // Null-terminate buffer
    serialBuffer[serialBufferPos] = '\0';

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, serialBuffer);

    // Send invalid json error
    if (err || !doc.is<JsonObject>()) {
      sendError("Invalid JSON");
      resetSerialBuffer();
      return;
    }

    // All event, location and payload keys must be present
    if (doc.size() != 3) {
      sendError("All 'event', 'location' and 'payload' keys are required");
      resetSerialBuffer();
      return;
    }

    // Only event, location and payload keys allowed
    for (JsonPair kv : doc.as<JsonObject>()) {
      const char *key = kv.key().c_str();
      if (strcmp(key, "event") != 0 && strcmp(key, "location") != 0 && strcmp(key, "payload") != 0) {
        sendError("Only 'event', 'location' and 'payload' keys are allowed");
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
      sendError("Invalid event 'post' for location 'battery'");
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
  Serial.println("get");
}

// Handler to run relevant endpoint function on POST
void UsbSerial::handlePost(JsonDocument &doc) {
  Serial.println("post");
}

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
