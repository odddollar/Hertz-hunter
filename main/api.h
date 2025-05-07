#ifndef API_H
#define API_H

#include <Arduino.h>
#include <WiFi.h>

class API {
public:
  API(char *s, char *pwd);
  void startWifi();
  void stopWifi();
  IPAddress getIP();
private:
  bool wifiOn;
  char* ssid;
  char* password;
};

#endif
