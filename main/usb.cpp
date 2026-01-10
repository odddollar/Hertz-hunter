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

// Start listening for commands
void UsbSerial::listen() {
  while (Serial.available()) {
    char c = Serial.read();

    // Ignore carriage return
    if (c == '\r') continue;

    // Not end of message
    if (c != '\n') {
      if (serialBufferPos < SERIAL_BUFFER_LENGTH - 1) {
        serialBuffer[serialBufferPos++] = c;
      }
    } else {
      // Null-terminate
      serialBuffer[serialBufferPos] = '\0';

      // Parse JSON
      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, serialBuffer);

      // Make sure json valid
      if (!err && doc.is<JsonObject>()) {
        Serial.println("Success!");
      } else {
        JsonDocument doc;

        doc["event"] = "error";
        doc["message"] = "Invalid JSON";

        send(doc);
      }

      // Reset state
      serialBufferPos = 0;
    }
  }
}
