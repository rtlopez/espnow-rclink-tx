#include "gamepad.hpp"

#ifdef USE_GAMEPAD
#include <Joystick_ESP32S2.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 3, 0,
                   true, true, true,    // axis x,y,z
                   false, false, false, // rotation x,y,z
                   true, true,          // throttle, rudder
                   false, false, false  // accel, brake, steering
);

constexpr int16_t INPUT_OFFSET = -1500;
constexpr int16_t INPUT_RANGE_MIN = -512;
constexpr int16_t INPUT_RANGE_MAX = 512;
constexpr int16_t AXIS_RANGE_MIN = -512;
constexpr int16_t AXIS_RANGE_MAX = 512;
constexpr int16_t BUTTON_THRESHOLD = 1650;

static inline int16_t gamepadScale(int16_t val) {
  return constrain(val + INPUT_OFFSET, INPUT_RANGE_MIN, INPUT_RANGE_MAX);
}
#endif

Gamepad::Gamepad(Tx &tx) : _tx(tx) {}

int Gamepad::begin() {
#ifdef USE_GAMEPAD
  if (_tx.getConfig().gamepad) {
    Joystick.setXAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setYAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setZAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setThrottleRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setRudderRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.begin(false);
    _tx.subscribe(EV_CHANNEL_UPDATE, [this]() {
      Joystick.setXAxis(gamepadScale(_tx.getChannel(0)));
      Joystick.setYAxis(gamepadScale(_tx.getChannel(1)));
      Joystick.setThrottle(gamepadScale(_tx.getChannel(2)));
      Joystick.setRudder(gamepadScale(_tx.getChannel(3)));
      Joystick.setZAxis(gamepadScale(_tx.getChannel(4)));
      // channel 6, 7, 8 as buttons
      for (size_t i = 0; i < 3; i++) {
        Joystick.setButton(i, _tx.getChannel(i + 5) >= BUTTON_THRESHOLD);
      }
      Joystick.sendState();
    });
  }
#endif
  return 0;
}

void Gamepad::end() {
#ifdef USE_GAMEPAD
  //TODO: _tx.unsubscribe()
  Joystick.end();
#endif
}