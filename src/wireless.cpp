#include "wireless.hpp"

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

const char *default_wifi_ssid = STR(WIFI_SSID);
const char *default_wifi_pass = STR(WIFI_PASS);

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
  WiFi.setHostname(_ssid);
  WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  WiFi.begin(default_wifi_ssid, default_wifi_pass);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(100);
  // }
  // Serial.println(WiFi.localIP());
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP(_ssid);

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

int Wireless::_updateConfig() {
  const auto &c = _tx.getConfig();
  String cfg;
  cfg.reserve(128);
  cfg = "{\"config\": {\"rxType\":";
  cfg += String(c.rxType);
  cfg += ",\"txType\":";
  cfg += String(c.txType);
  cfg += ",\"rxPin\":";
  cfg += String(c.rxPin);
  cfg += ",\"txPin\":";
  cfg += String(c.txPin);
  cfg += ",\"adc0\":";
  cfg += String(c.analogPins[0]);
  cfg += ",\"adc1\":";
  cfg += String(c.analogPins[1]);
  cfg += ",\"adc2\":";
  cfg += String(c.analogPins[2]);
  cfg += ",\"adc3\":";
  cfg += String(c.analogPins[3]);
  cfg += ",\"btn0\":";
  cfg += String(c.buttonPins[0]);
  cfg += ",\"btn1\":";
  cfg += String(c.buttonPins[1]);
  cfg += ",\"btn2\":";
  cfg += String(c.buttonPins[2]);
  cfg += ",\"btn3\":";
  cfg += String(c.buttonPins[3]);
  cfg += ",\"gamepad\":";
  cfg += String(c.gamepad);
  cfg += ",\"debug\":";
  cfg += String(c.debug);
  cfg += "}}";
  _events.send(cfg.c_str(), "message", _counter++);
  return 1;
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
  _events.send(m.c_str(), "message", _counter++);
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
  _events.send(m.c_str(), "message", _counter++);
  return 1;
}

void Wireless::end() {
  MDNS.end();
  _server.end();
  WiFi.softAPdisconnect(true);
}
