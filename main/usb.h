#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "battery.h"
#include "RX5808.h"
#include "settings.h"

#define USB_SERIAL_BAUD 115200

#define SERIAL_BUFFER_LENGTH 256

// Holds state and responses usb serial communication
class UsbSerial {
public:
#ifdef BATTERY_MONITORING
  UsbSerial(Settings *s, RX5808 *r, Battery *b);
#else
  UsbSerial(Settings *s, RX5808 *r);
#endif
  void beginSerial(unsigned long baud);
  void flushIncoming();
  void listen();

private:
  void handleGet(JsonDocument &doc);
  void handlePost(JsonDocument &doc);
  void handleGetValues();
  void handlePostValues(JsonDocument &doc);
  void handleGetSettings();
  void handlePostSettings(JsonDocument &doc);
  void handleGetCalibration();
  void handlePostCalibration(JsonDocument &doc);
#ifdef BATTERY_MONITORING
  void handleGetBattery();
#endif
  void sendJson(JsonDocument &doc);
  void sendError(const char *location, const char *msg);
  void resetSerialBuffer();

  bool serialOn;

  char serialBuffer[SERIAL_BUFFER_LENGTH];
  int serialBufferPos;
  bool serialBufferOverflow;

  Settings *settings;
  RX5808 *receiver;
#ifdef BATTERY_MONITORING
  Battery *battery;
#endif
};

#endif
