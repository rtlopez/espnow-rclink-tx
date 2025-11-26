#pragma once

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

using TxEventCallback = std::function<void()>;

struct TxCallbackHolder {
  TxEvent e;
  TxEventCallback cb;
};

class EventEmiter {
public:
  EventEmiter();
  void subscribe(TxEvent e, TxEventCallback &&cb);
  void dispatch(TxEvent e);

private:
  std::vector<TxCallbackHolder> _subscribers;
};