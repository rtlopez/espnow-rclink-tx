#pragma once

#include "tx.hpp"
#include <cstddef>
#include <cstdint>

class Sim {
public:
  Sim(Tx &tx);

  void begin();
  void end();
  int get(size_t channel) const;
  int update();

private:
  int _generate(int time);

  static constexpr uint32_t CYCLE_TIME = 8000;    // 8 seconds
  static constexpr uint32_t SIM_INTERVAL_MS = 20; // 20ms => 50Hz

  Tx &_tx;
  uint32_t _tm = 0;
  size_t _ch = 0;
  int _val[16];
};
