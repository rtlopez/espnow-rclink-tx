#include "espnow.hpp"

EspNowLink::EspNowLink(Tx &tx) : _tx(tx) {}

int EspNowLink::begin() {
  TxConfig &c = _tx.getConfig();

  if (c.txType == MODE_ESPNOW) {
    _txEspNow.begin(true);
    _tx.subscribe(EV_CHANNEL_UPDATE, [this]() {
      for (size_t channel = 0; channel < 8; ++channel) {
        _txEspNow.setChannel(channel, _tx.getChannel(channel));
      }
      _txEspNow.commit();
    });
  } else if (c.rxType == MODE_ESPNOW) {
    _rxEspNow.begin(true);
  }

  return 0;
}

void EspNowLink::end() {
  // TODO: _tx.unsubscribe()
  //_txEspNow.end();
  //_rxEspNow.end();
}

int EspNowLink::update() {
  uint32_t now = micros();
  TxConfig &c = _tx.getConfig();

  if (c.rxType == MODE_ESPNOW && c.txType != MODE_ESPNOW) {
    _rxEspNow.update();
    if (_rxEspNow.available()) {
      for (size_t channel = 0; channel < 8; ++channel) {
        _tx.setChannel(channel, _rxEspNow.getChannel(channel));
      }
      _tx.setAvailable(now);
    }
  }

  return 0;
}
