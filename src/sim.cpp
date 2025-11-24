#include "sim.hpp"
#include <Arduino.h>
#include <algorithm>

Sim::Sim(Tx &tx) : _tx(tx) { std::fill_n(_val, 16, 0); }

void Sim::begin() { _tm = millis() % CYCLE_TIME; }

int Sim::get(size_t channel) const { return 1500 + _val[channel]; }

int Sim::update() {
  static uint32_t simNext = 0;
  uint32_t now = micros();
  if (now > simNext) {
    uint32_t time = (now / 1000) % CYCLE_TIME;

    // detect overflow to cycle over channels
    if (time < _tm) {
      _ch++;
      _ch %= 4;
    }
    _tm = time;
    _val[_ch] = _generate(time);

    _tx.update(*this);
    _tx.setAvailable(now);
    simNext = now + SIM_INTERVAL_MS * 1000;
    return 1;
  }
  return 0;
}

int Sim::_generate(int time) {
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