#pragma once

#include "tx.hpp"
#include <Arduino.h>
#include <functional>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class Wireless {
public:
  Wireless(Tx &tx, const char *ssid = "espnow-rclink-tx");
  int begin();

  int update();

  IPAddress getApIp();
  void end();

private:
  int _updateChannels();
  int _updateCalibration();

  Tx &_tx;
  const char *_ssid;
  int _counter;
  AsyncWebServer _server;
  AsyncEventSource _events;
};
