#pragma once

#include "tx.hpp"
#include <Arduino.h>

class Monitor {
public:
  Monitor(Tx &tx) : _tx(tx) {}

  int begin() {
    if (_tx.getConfig().debug) {
      Serial.begin(115200);
      _tx.subscribe(EV_CHANNEL_UPDATE, [this]() {
        if (_lastRcv + 1000000 > micros()) {
          _lastRcv = micros();
          Serial.printf("cn %d,%d,%d,%d,%d\n", _tx.getChannel(0),
                        _tx.getChannel(1), _tx.getChannel(2), _tx.getChannel(3),
                        _tx.getChannel(4));
        }
      });
      _tx.subscribe(EV_CALIBRATION_UPDATE, [this]() {
        if (_lastCal + 1000000 > micros()) {
          _lastCal = micros();
          for (size_t i = 0; i < 4; i++) {
            auto [l, m, h] = _tx.getCalibration(i);
            Serial.printf("cl %d,%d,%d,%d\n", i, l, m, h);
          }
        }
      });
      _tx.subscribe(EV_FAILSAFE_ENTER,
                    [this]() { Serial.println("failsafe enter"); });
      _tx.subscribe(EV_FAILSAFE_EXIT,
                    [this]() { Serial.println("failsafe exit"); });
    }
    return 0;
  }

private:
  Tx &_tx;
  uint32_t _lastRcv = 0;
  uint32_t _lastCal = 0;
};
