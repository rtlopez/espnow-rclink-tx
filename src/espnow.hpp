#pragma once

#include "tx.hpp"
#include <EspNowRcLink/Receiver.h>
#include <EspNowRcLink/Transmitter.h>

class EspNowLink {
public:
  EspNowLink(Tx &tx);
  int begin();
  void end();
  int update();

  void onChannelUpdate();

private:
  Tx &_tx;
  EspNowRcLink::Receiver _rxEspNow;
  EspNowRcLink::Transmitter _txEspNow;
};
