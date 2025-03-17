#pragma once
#include <cstdint>
#include <cstddef>
#include <Arduino.h>

class Analog
{
public:
  enum PinMode { MODE_OFF, MODE_ANALOG, MODE_DIGITAL };

  Analog() {};

  void attach(size_t channel, PinMode mode, int8_t pin)
  {
    _input[channel].mode = mode;
    _input[channel].pin = pin;
    _input[channel].value = 0;
    if(pin == -1) return;
    switch(mode)
    {
      case MODE_DIGITAL:
        pinMode(pin, INPUT_PULLUP);
        break;

      case MODE_ANALOG:
        pinMode(pin, ANALOG);
        break;

      case MODE_OFF:
      default:
        break;
    }
  }

  void update()
  {
    for(auto& in: _input)
    {
      if(in.pin == -1) continue;
      switch(in.mode)
      {
        case MODE_DIGITAL:
          in.value = digitalRead(in.pin) ? 0 : 4096;
          break;

        case MODE_ANALOG:
          in.value = analogRead(in.pin);
          break;

        case MODE_OFF:
        default:
          break;
      }
    }
  }

  int get(size_t channel) const
  {
    return map(_input[channel].value, 0, 4095, 1000, 2000);
  }

private:
  struct {
    PinMode mode = MODE_OFF;
    int pin = -1;
    int value = 0;
  } _input[16];
};
