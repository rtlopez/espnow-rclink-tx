#include "analog.hpp"
#include <Arduino.h>
#include <algorithm>

Analog::Analog(Tx &tx) : _tx(tx) {}

int Analog::begin() {
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
  TxConfig &c = _tx.getConfig();
  attach(0, Analog::PIN_ANALOG, c.analogPins[0]);
  attach(1, Analog::PIN_ANALOG, c.analogPins[1]);
  attach(2, Analog::PIN_ANALOG, c.analogPins[2]);
  attach(3, Analog::PIN_ANALOG, c.analogPins[3]);
  attach(4, Analog::PIN_DIGITAL, c.buttonPins[0]);
  attach(5, Analog::PIN_DIGITAL, c.buttonPins[1]);
  attach(6, Analog::PIN_DIGITAL, c.buttonPins[2]);
  attach(7, Analog::PIN_DIGITAL, c.buttonPins[3]);
  return 1;
}

void Analog::end() {
  for(auto i: _input) {
    if(i.pin != -1) {
      pinMode(i.pin, INPUT);
    }
  }
}

void Analog::attach(size_t channel, PinMode mode, int8_t pin) {
  _input[channel].mode = mode;
  _input[channel].pin = pin;
  _input[channel].value = 0;
  if (pin == -1)
    return;
  switch (mode) {
  case PIN_DIGITAL:
    pinMode(pin, INPUT_PULLUP);
    break;
  case PIN_ANALOG:
    pinMode(pin, ANALOG);
    break;
  case PIN_OFF:
  default:
    break;
  }
}

int Analog::update() {
  static uint32_t analogNext = 0;
  uint32_t now = micros();
  if (now > analogNext) {
    analogNext = now + 20000;
    for (auto &in : _input) {
      if (in.pin == -1)
        continue;
      switch (in.mode) {
      case PIN_DIGITAL:
        in.value = !digitalRead(in.pin);
        break;
      case PIN_ANALOG:
        in.value = analogRead(in.pin);
        break;
      case PIN_OFF:
      default:
        break;
      }
    }
    if (_tx.isCalibrationEnabled()) {
      _calibrate();
    }
    _tx.update(*this);
    _tx.setAvailable(now);
    return 1;
  }
  return 0;
}

int Analog::get(size_t channel) const {
  auto &c = _input[channel];
  if (channel < 4) {
    auto [inMin, inMid, inMax] = _tx.getCalibration(channel);
    if (_tx.isCalibrationEnabled() && (inMax - inMin < 200)) {
      return 1500;
    }
    auto val = _scale(c.value, inMin, inMid, inMax, 1000, 1500, 2000);
    return std::clamp(val, 1000, 2000);
  } else {
    return c.value ? 2000 : 1000;
  }
}

void Analog::_calibrate() {
  for (size_t i = 0; i < 4; ++i) {
    auto &c = _input[i];
    auto [inMin, inMid, inMax] = _tx.getCalibration(i);
    inMin = std::min(inMin, c.value);
    inMax = std::max(inMax, c.value);
    c.centerMin = ((inMin + inMax) * 2) / 6;
    c.centerMax = ((inMin + inMax) * 3) / 6;
    c.center += 0.1f * (c.value - c.center);
    c.center = std::clamp(c.center, (float)c.centerMin, (float)c.centerMax);
    inMid = (int)c.center;
    _tx.setCalibration(i, inMin, inMid, inMax);
  }
}

int Analog::_scale(int x, int in_min, int in_mid, int in_max, int out_min,
                   int out_mid, int out_max) const {
  return x >= in_mid ? _scale(x, in_mid, in_max, out_mid, out_max)
                     : _scale(x, in_min, in_mid, out_min, out_mid);
}

int Analog::_scale(int x, int in_min, int in_max, int out_min,
                   int out_max) const {
  const int run = in_max - in_min;
  if (run == 0)
    return out_min;
  const int rise = out_max - out_min;
  const int delta = x - in_min;
  return (delta * rise + run / 2) / run + out_min;
}