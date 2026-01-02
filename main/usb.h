#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "battery.h"
#include "RX5808.h"
#include "settings.h"

#define USB_SERIAL_BAUD 115200

// Holds state and responses usb serial communication
class UsbSerial {
public:
#ifdef BATTERY_MONITORING
  UsbSerial(Settings *s, RX5808 *r, Battery *b);
#else
  UsbSerial(Settings *s, RX5808 *r);
#endif
  void beginSerial(unsigned long baud);
  void listen();

private:
  bool serialOn;

  Settings *settings;
  RX5808 *module;
  Battery *battery;
};

#endif
