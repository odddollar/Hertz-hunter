#ifndef API_H
#define API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "battery.h"
#include "RX5808.h"
#include "settings.h"

#define WIFI_SSID "Hertz Hunter"
#define WIFI_PASSWORD "hertzhunter"
#define WIFI_IP "192.168.4.1"
#define WIFI_GATEWAY WIFI_IP
#define WIFI_SUBNET "255.255.255.0"

// Holds state and responses for wifi and api
class Api {
public:
  Api(Settings *s, RX5808 *r, Battery *b);
  void startWifi();
  void stopWifi();

private:
  void handleNotFound(AsyncWebServerRequest *request);
  void handleGetBattery(AsyncWebServerRequest *request);
  void handleGetValues(AsyncWebServerRequest *request);
  void handlePostValues(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleGetSettings(AsyncWebServerRequest *request);
  void handlePostSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleGetCalibration(AsyncWebServerRequest *request);
  void handlePostCalibration(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

  bool wifiOn;

  AsyncWebServer server;

  Settings *settings;
  RX5808 *module;
  Battery *battery;
};

#endif
