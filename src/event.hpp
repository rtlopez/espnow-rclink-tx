#pragma once

#include <algorithm>
#include <functional>
#include <vector>

enum TxEvent : uint8_t {
  EV_CHANNEL_UPDATE,
  EV_FAILSAFE_ENTER,
  EV_FAILSAFE_EXIT,
  EV_CALIBRATION_START,
  EV_CALIBRATION_UPDATE,
  EV_CALIBRATION_DONE,
  EV_SAVE,
};

struct TxCallbackEntry {
  std::function<void(TxEvent)> caller;
  std::function<bool(void* oPtr)> matcher;
};

class EventEmiter {
public:
  EventEmiter();

  template <typename T>
  void subscribe(TxEvent ev, void (T::*fn)(), T* obj) {
    _subscribers.push_back({
      [ev, fn, obj](TxEvent e) { if(ev == e) (static_cast<T*>(obj)->*fn)(); },
      [obj](void* oPtr) { return obj == oPtr; }
    });
  }

  template <typename T>
  void unsubscribe(T *obj) {
    auto it = std::remove_if(
      _subscribers.begin(),
      _subscribers.end(),
      [obj](const TxCallbackEntry &h) { return h.matcher(obj); }
    );
    _subscribers.erase(it, _subscribers.end());
  }

  void dispatch(TxEvent ev) {
    for (auto &h : _subscribers) {
      h.caller(ev);
    }
  }

private:
  std::vector<TxCallbackEntry> _subscribers;
};