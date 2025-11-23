#include "wireless.hpp"

extern const uint8_t
    wireless_index[] asm("_binary_src_wireless_index_html_start");
extern const uint8_t
    wireless_index_end[] asm("_binary_src_wireless_index_html_end");

Wireless::Wireless(Tx &tx, const char *ssid)
    : _tx(tx), _ssid(ssid), _counter(0), _server(80), _events("/events") {}

int Wireless::begin() {
  // WiFi.begin(ssid, pass);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  // }
  // Serial.println(WiFi.localIP());

  WiFi.mode(WIFI_AP);
  WiFi.setHostname(_ssid);
  WiFi.softAP(_ssid);
  MDNS.begin(_ssid);

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
                  reinterpret_cast<const char *>(wireless_index));
  });

  _server.on("/analog/calibrate/start", HTTP_POST,
             [this](AsyncWebServerRequest *request) {
               // if (_callback) _callback(CALIBRATION_START);
               _tx.enableCalibration();
               request->send(200, "text/html", "");
             });

  _server.on("/analog/calibrate/save", HTTP_POST,
             [this](AsyncWebServerRequest *request) {
               // if (_callback) _callback(CALIBRATION_SAVE);
               _tx.save();
               request->send(200, "text/html", "");
             });

  _events.onConnect([this](AsyncEventSourceClient *client) {
    client->send("{\"connected\":true}", "message", _counter++);
  });

  _server.addHandler(&_events);
  _server.begin();

  return 1;
}

IPAddress Wireless::getApIp() { return WiFi.softAPIP(); }

int Wireless::update() {
  uint32_t now = micros();

  static uint32_t channelsNext = 0;
  if (now > channelsNext) {
    channelsNext = now + 200000ul;
    _updateChannels();
  }

  static bool prevCalibration = false;
  bool currCalibration = _tx.isCalibrationEnabled();
  bool statusChanged = prevCalibration != currCalibration;
  static uint32_t calibrationNext = 0;
  if (statusChanged || (currCalibration && now > calibrationNext)) {
    calibrationNext = now + 500000ul;
    _updateCalibration();
    if (statusChanged) {
      prevCalibration = currCalibration;
    }
  }

  return 0;
}

int Wireless::_updateChannels() {
  String m;
  m.reserve(128);
  m += "{\"channels\":[";
  for (size_t i = 0; i < 8; i++) {
    if (i) {
      m += ",";
    }
    m += _tx.getChannel(i);
  }
  m += "]}";
  _events.send(m.c_str(), "message", _counter);
  _counter++;
  return 1;
}

int Wireless::_updateCalibration() {
  String m;
  m.reserve(128);
  m += "{\"calibration\":[";
  for (size_t i = 0; i < 4; i++) {
    auto [l, c, h] = _tx.getCalibration(i);
    if (i) {
      m += ",";
    }
    m += "[";
    m += l;
    m += ",";
    m += c;
    m += ",";
    m += h;
    m += "]";
  }
  m += "], \"active\": ";
  m += _tx.isCalibrationEnabled() ? "true" : "false";
  m += "}";
  _events.send(m.c_str(), "message", _counter);
  _counter++;
  return 1;
}

void Wireless::end() {
  MDNS.end();
  _server.end();
  WiFi.softAPdisconnect(true);
}
