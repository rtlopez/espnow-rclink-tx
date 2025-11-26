#pragma once

#include "tx.hpp"

class Gamepad {
public:
  Gamepad(Tx &tx);
  int begin();
  void end();

private:
  Tx &_tx;
};
