#ifndef API_H
#define API_H

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "RX5808.h"
#include "settings.h"

#define SSID "Hertz Hunter"
#define PASSWORD "hertzhunter"

// Holds state and responses for wifi and api
class Api {
public:
  Api(Settings *s, RX5808 *r);
  void startWifi();
  void stopWifi();

private:
  bool wifiOn;

  AsyncWebServer server;

  Settings *settings;
  RX5808 *module;
};

#endif
