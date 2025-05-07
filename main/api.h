#ifndef API_H
#define API_H

#include <Arduino.h>
#include <WiFi.h>

class API {
public:
  API(const char *s, const char *pwd);
  void startWifi();
  void stopWifi();
  IPAddress getIP();
private:
  bool wifiOn;
  const char* ssid;
  const char* password;
};

#endif
