#pragma once

#include <cstddef>
#include <cstdint>
#include <Arduino.h>

class Sim
{
public:
  Sim()
  {
    std::fill_n(_val, 16, 0);
  }

  void begin()
  {
    _tm = millis() % CYCLE_TIME;
  }

  int get(size_t channel) const
  {
    return 1500 + _val[channel];
  }

  void update()
  {
    uint32_t time = millis() % CYCLE_TIME;

    // detect overflow to cycle over channels
    if(time < _tm)
    {
      _ch++;
      _ch %= 4;
    }
    _tm = time;

    _val[_ch] = _generate(time);
  }

private:
  int _generate(int time)
  {
    if (time < 1000) {
      return (time * 500) / 1000; // Increment 0 -> 500 for 1 sec
    } else if (time < 2000) {
        return 500; // Hold 500 for 1 sec
    } else if (time < 3000) {
        return 500 - ((time - 2000) * 500) / 1000; // Decrement 500 -> 0 for 1 sec
    } else if (time < 4000) {
        return 0; // Hold 1 sec
    } else if (time < 5000) {
        return -((time - 4000) * 500) / 1000; // Decrement 0 -> -500 for 1 sec
    } else if (time < 6000) {
        return -500; // Hold -500 for 1 sec
    } else if (time < 7000) {
        return -500 + ((time - 6000) * 500) / 1000; // Increment -500 -> 0 for 1 sec
    } else {
        return 0; // Hold 1 sec
    }
  }

  static constexpr uint32_t CYCLE_TIME = 8000; // 8 seconds

  uint32_t _tm = 0;
  size_t _ch = 0;
  int _val[16];
};

