#ifndef API_H
#define API_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class API {
public:
  API(const char* s, const char* pwd, int (*rssiV)[61], int* nSV, int* bV, int (*sI)[3], int (*cRSSI)[2], SemaphoreHandle_t m);
  void startWifi();
  void stopWifi();
  const char* getIP();
private:
  bool wifiOn;
  const char* ssid;
  const char* password;
  int (*rssiValues)[61];
  int* numScannedValues;
  int* batteryVoltage;
  int (*settingsIndices)[3];
  int (*calibratedRssi)[2];
  SemaphoreHandle_t mutex;
  AsyncWebServer server;
};

#endif
