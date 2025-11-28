#pragma once

#include "tx.hpp"
#include <Arduino.h>
#include <functional>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class Wireless {
public:
  Wireless(Tx &tx, const char *ssid = "espnow-rclink-tx");
  int begin();

  int update();

  IPAddress getApIp();
  void end();

private:
  int _updateConfig();
  int _updateChannels();
  int _updateCalibration();
  void _sendJson(const JsonDocument& json);

  Tx &_tx;
  const char *_ssid;
  int _counter;
  AsyncWebServer _server;
  AsyncEventSource _events;
};
