#pragma once

#include "tx.hpp"
#include <Arduino.h>

class Monitor {
public:
  Monitor(Tx &tx) : _tx(tx) {}

  int begin() {
    Serial.begin(115200);
    if (_tx.getConfig().debug) {
      _tx.subscribe(EV_CHANNEL_UPDATE, &Monitor::onChannelUpdate, this);
      _tx.subscribe(EV_CALIBRATION_UPDATE, &Monitor::onCalibrationUpdate, this);
      _tx.subscribe(EV_FAILSAFE_ENTER, &Monitor::onFailSafeEnter, this);
      _tx.subscribe(EV_FAILSAFE_EXIT, &Monitor::onFailSafeExit, this);
    }
    return 0;
  }

  void end() {
    _tx.unsubscribe(this);
  }

  void onChannelUpdate() {
    if (_lastRcv + 1000000 > micros()) {
      _lastRcv = micros();
      Serial.printf("cn %d,%d,%d,%d,%d\n", _tx.getChannel(0), _tx.getChannel(1),
                    _tx.getChannel(2), _tx.getChannel(3), _tx.getChannel(4));
    }
  }

  void onCalibrationUpdate() {
    if (_lastCal + 1000000 > micros()) {
      _lastCal = micros();
      for (size_t i = 0; i < 4; i++) {
        auto [l, m, h] = _tx.getCalibration(i);
        Serial.printf("cl %d,%d,%d,%d\n", i, l, m, h);
      }
    }
  }

  void onFailSafeEnter() { Serial.println("failsafe enter"); }

  void onFailSafeExit() { Serial.println("failsafe exit"); }

private:
  Tx &_tx;
  uint32_t _lastRcv = 0;
  uint32_t _lastCal = 0;
};
