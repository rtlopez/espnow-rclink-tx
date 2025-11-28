#pragma once

#include "tx.hpp"
#include <cstddef>
#include <cstdint>

class Analog {
public:
  enum PinMode { PIN_OFF, PIN_ANALOG, PIN_DIGITAL };

  Analog(Tx &tx);

  int begin();
  void end();
  void attach(size_t channel, PinMode mode, int8_t pin);
  int update();
  int get(size_t channel) const;

private:
  void _calibrate();

  int _scale(int x, int in_min, int in_mid, int in_max, int out_min,
             int out_mid, int out_max) const;

  int _scale(int x, int in_min, int in_max, int out_min, int out_max) const;

  struct ChannelData {
    PinMode mode = PIN_OFF;
    int pin = -1;
    int value = 0;
    float center = 2047;
    int centerMin = 0;
    int centerMax = 4095;
  } _input[16];

  Tx &_tx;
};
