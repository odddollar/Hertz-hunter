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

// Send json to serial
void UsbSerial::send(JsonDocument &doc) {
  serializeJson(doc, Serial);
  Serial.println();
}

// Send json error message
void UsbSerial::sendError(const char *msg) {
  JsonDocument doc;

  doc["event"] = "error";
  doc["message"] = msg;

  send(doc);
}

// Reset serial buffer state
void UsbSerial::resetSerialBuffer() {
  serialBufferPos = 0;
  serialBufferOverflow = false;
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

    if (!err && doc.is<JsonObject>()) {
      Serial.println("Success!");
    } else {
      sendError("Invalid JSON");
    }

    // Reset for next message
    resetSerialBuffer();
  }
}
