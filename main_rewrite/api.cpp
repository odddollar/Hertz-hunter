#include "api.h"

Api::Api(Settings *s, RX5808 *r)
  : wifiOn(false),
    settings(s), module(r),
    server(80) {

  // 404 endpoint
  server.onNotFound(notFound);
}

// Start wifi hotspot
void Api::startWifi() {
  // Do nothing if wifi already on
  if (wifiOn) return;

  // Set static ip
  IPAddress ip, gateway, subnet;
  ip.fromString(WIFI_IP);
  gateway.fromString(WIFI_GATEWAY);
  subnet.fromString(WIFI_SUBNET);
  WiFi.softAPConfig(ip, gateway, subnet);

  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  server.begin();

  wifiOn = true;
}

// Stop wifi hotspot
void Api::stopWifi() {
  // Do nothing if wifi already off
  if (!wifiOn) return;

  server.end();
  WiFi.softAPdisconnect(true);

  wifiOn = false;
}

// Return 404 not found error
void Api::notFound(AsyncWebServerRequest *request) {
  JsonDocument doc;

  doc["error"] = "404: Not found";
  doc["path"] = request->url();

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->setCode(404);

  serializeJson(doc, *response);
  request->send(response);
}
