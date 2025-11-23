#include "analog.hpp"
#include <Arduino.h>
#include <algorithm>

Analog::Analog(Tx& tx) : _tx(tx) {}

void Analog::begin() {
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
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

void Analog::update() {
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
  if (_tx.isCalibrationEnabled())
    _calibrate();
}

int Analog::get(size_t channel) const {
  auto &c = _input[channel];
  if (channel < 4) {
    auto [inMin, inMid, inMax] = _tx.getCalibration(channel);
    if (_tx.isCalibrationEnabled() && (inMax - inMin < 200)) return 1500;
    return std::clamp(
        _scale(c.value, inMin, inMid, inMax, 1000, 1500, 2000),
        1000, 2000);
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

int Analog::_scale(int x, int in_min, int in_mid, int in_max, int out_min, int out_mid, int out_max) const {
  return x >= in_mid ? _scale(x, in_mid, in_max, out_mid, out_max)
                     : _scale(x, in_min, in_mid, out_min, out_mid);
}

int Analog::_scale(int x, int in_min, int in_max, int out_min, int out_max) const {
  const int run = in_max - in_min;
  if (run == 0) return out_min;
  const int rise = out_max - out_min;
  const int delta = x - in_min;
  return (delta * rise + run / 2) / run + out_min;
}