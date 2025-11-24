#pragma once

#include "tx.hpp"

class Gamepad {
public:
  Gamepad(Tx &tx);
  int begin();

private:
  Tx &_tx;
};
