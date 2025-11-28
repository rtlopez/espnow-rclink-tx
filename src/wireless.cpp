#include "wireless.hpp"
#include <algorithm>
#include <cstring>

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

const char *force_wifi_ssid = STR(WIFI_SSID);
const char *force_wifi_pass = STR(WIFI_PASS);

extern const uint8_t
    wireless_index[] asm("_binary_src_wireless_index_html_start");
extern const uint8_t
    wireless_index_end[] asm("_binary_src_wireless_index_html_end");

Wireless::Wireless(Tx &tx, const char *ssid)
    : _tx(tx), _ssid(ssid), _counter(0), _server(80), _events("/events") {}

static void WiFiGotIP(arduino_event_id_t event, arduino_event_info_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

int Wireless::begin() {
  TxConfig &c = _tx.getConfig();
  WiFi.setHostname(_ssid);
  WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  if (std::strlen(force_wifi_ssid)) {
    WiFi.begin(force_wifi_ssid, force_wifi_pass);
  } else if (std::strlen(c.wifiSsid)) {
    WiFi.begin(c.wifiSsid, c.wifiPass);
  }
  WiFi.softAP(_ssid);
  MDNS.begin(_ssid);

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
                  reinterpret_cast<const char *>(wireless_index));
  });

  _server.on("/analog/calibrate/start", HTTP_POST,
             [this](AsyncWebServerRequest *request) {
               _tx.enableCalibration();
               request->send(200, "text/html", "");
             });

  _server.on("/analog/calibrate/save", HTTP_POST,
             [this](AsyncWebServerRequest *request) {
               _tx.save();
               request->send(200, "text/html", "");
             });

  _server.on("/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
    auto count = request->params();
    for (size_t i = 0; i < count; i++) {
      const auto *p = request->getParam(i);
      auto &c = _tx.getConfig();
      const String &name = p->name();
      const String &value = p->value();
      if (name == "rxType") {
        c.rxType = static_cast<RxTxMode>(value.toInt());
      } else if (name == "txType") {
        c.txType = static_cast<RxTxMode>(value.toInt());
      } else if (name == "rxPin") {
        c.rxPin = value.toInt();
      } else if (name == "txPin") {
        c.txPin = value.toInt();
      } else if (name == "adc0") {
        c.analogPins[0] = value.toInt();
      } else if (name == "adc1") {
        c.analogPins[1] = value.toInt();
      } else if (name == "adc2") {
        c.analogPins[2] = value.toInt();
      } else if (name == "adc3") {
        c.analogPins[3] = value.toInt();
      } else if (name == "btn0") {
        c.buttonPins[0] = value.toInt();
      } else if (name == "btn1") {
        c.buttonPins[1] = value.toInt();
      } else if (name == "btn2") {
        c.buttonPins[2] = value.toInt();
      } else if (name == "btn3") {
        c.buttonPins[3] = value.toInt();
      } else if (name == "gamepad") {
        c.gamepad = (value == "on");
      } else if (name == "debug") {
        c.debug = (value == "on");
      } else if (name == "wifiSsid") {
        const size_t len = std::min(value.length(), 32u);
        std::copy_n(value.c_str(), len, c.wifiSsid);
        c.wifiSsid[len] = '\0';
      } else if (name == "wifiPass") {
        const size_t len = std::min(value.length(), 32u);
        std::copy_n(value.c_str(), len, c.wifiPass);
        c.wifiPass[len] = '\0';
      }
    }
    _tx.save();
    _updateConfig();
    request->send(200, "text/html", "");
  });

  _events.onConnect([this](AsyncEventSourceClient *client) {
    client->send("{\"connected\":true}", "message", _counter++);
    _updateConfig();
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

void Wireless::_sendJson(const JsonDocument& json)
{
  String m;
  m.reserve(128);
  serializeJson(json, m);
  _events.send(m.c_str(), "message", _counter++);
}

int Wireless::_updateConfig() {
  const auto &c = _tx.getConfig();
  JsonDocument doc;
  auto config = doc["config"].to<JsonObject>();
  config["rxType"] = c.rxType;
  config["txType"] = c.txType;
  config["rxPin"] = c.rxPin;
  config["txPin"] = c.txPin;
  config["adc0"] = c.analogPins[0];
  config["adc1"] = c.analogPins[1];
  config["adc2"] = c.analogPins[2];
  config["adc3"] = c.analogPins[3];
  config["btn0"] = c.buttonPins[0];
  config["btn1"] = c.buttonPins[1];
  config["btn2"] = c.buttonPins[2];
  config["btn3"] = c.buttonPins[3];
  config["gamepad"] = c.gamepad;
  config["debug"] = c.debug;
  config["wifiSsid"] = c.wifiSsid;
  config["wifiPass"] = c.wifiPass; 
  _sendJson(doc);
  return 1;
}

int Wireless::_updateChannels() {
  JsonDocument doc;
  auto channels = doc["channels"].to<JsonArray>();
  for (size_t i = 0; i < 8; i++) {
    channels.add(_tx.getChannel(i));
  }
  _sendJson(doc);
  return 1;
}

int Wireless::_updateCalibration() {
  JsonDocument doc;
  auto calibration = doc["calibration"].to<JsonArray>();
  for (size_t i = 0; i < 4; i++) {
    auto [l, c, h] = _tx.getCalibration(i);
    auto row = calibration.add<JsonArray>();
    row.add(l);
    row.add(c);
    row.add(h);
  }
  doc["active"] = _tx.isCalibrationEnabled();
  _sendJson(doc);
  return 1;
}

void Wireless::end() {
  MDNS.end();
  _server.end();
  WiFi.softAPdisconnect(true);
}
