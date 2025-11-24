#include "event.hpp"

EventEmiter::EventEmiter() {}

void EventEmiter::subscribe(TxEvent e, TxEventCallback &&cb) {
  _subscribers.push_back({e, std::move(cb)});
}

void EventEmiter::dispatch(TxEvent e) {
  for (auto &cbh : _subscribers) {
    if (cbh.e == e && cbh.cb)
      cbh.cb();
  }
}
