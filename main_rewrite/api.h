#ifndef API_H
#define API_H

#include <WiFi.h>
#include "settings.h"

#define SSID "Hertz Hunter"
#define PASSWORD "hertzhunter"

// Holds state and responses for wifi and api
class Api {
public:
  Api(Settings *s);
  void startWifi();
  void stopWifi();

private:
  bool wifiOn;

  Settings *settings;
};

#endif
