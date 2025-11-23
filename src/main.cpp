#include "EspNowRcLink/Receiver.h"
#include "EspNowRcLink/Transmitter.h"
#include "analog.hpp"
#include "ppm.h"
#include "sim.hpp"
#include "tx.hpp"
#include "wireless.hpp"
#include <Arduino.h>

Tx txData;

Sim rxSim;
PPM rxPpm;
Analog rxAnalog(txData);
EspNowRcLink::Receiver rxEspNow;
EspNowRcLink::Transmitter txEspNow;
Wireless wireless(txData, "espnow-rclink-tx");

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

int16_t gamepadScale(int16_t val) {
  return constrain(val + INPUT_OFFSET, INPUT_RANGE_MIN, INPUT_RANGE_MAX);
}
#endif

void setup() {
  txData.begin();
  TxConfig &c = txData.getConfig();

  if (c.debug) {
    Serial.begin(115200);
  }
  if (c.rxType == MODE_SIM) {
    rxSim.begin();
  }
  if (c.rxType == MODE_PPM) {
    rxPpm.begin(c.rxPin, FALLING);
  }

#ifdef USE_ANALOG
  if (c.rxType == MODE_ANALOG) {
    rxAnalog.begin();
    rxAnalog.attach(0, Analog::PIN_ANALOG, c.analogPins[0]);
    rxAnalog.attach(1, Analog::PIN_ANALOG, c.analogPins[1]);
    rxAnalog.attach(2, Analog::PIN_ANALOG, c.analogPins[2]);
    rxAnalog.attach(3, Analog::PIN_ANALOG, c.analogPins[3]);
    rxAnalog.attach(4, Analog::PIN_DIGITAL, c.buttonPins[0]);
    rxAnalog.attach(5, Analog::PIN_DIGITAL, c.buttonPins[1]);
    rxAnalog.attach(6, Analog::PIN_DIGITAL, c.buttonPins[2]);
    rxAnalog.attach(7, Analog::PIN_DIGITAL, c.buttonPins[3]);
  }
#endif

  if (c.rxType == MODE_ESPNOW) {
    rxEspNow.begin(true);
  }

  if (c.txType == MODE_ESPNOW) {
    txEspNow.begin(true);
  }

#ifdef USE_GAMEPAD
  if (c.gamepad) {
    Joystick.setXAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setYAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setZAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setThrottleRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.setRudderRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
    Joystick.begin(false);
  }
#endif

  wireless.begin();
}

template <typename Dst, typename Src> void update(Dst &dst, const Src &src) {
  for (size_t channel = 0; channel < 8; ++channel) {
    dst.setChannel(channel, src.get(channel));
  }
}

void loop() {
  uint32_t now = micros();
  TxConfig &c = txData.getConfig();

  // receive inputs

  static uint32_t simNext = 0;
  if (c.rxType == MODE_SIM) {
    constexpr uint32_t SIM_INTERVAL_MS = 20; // 20ms => 50Hz
    if (now > simNext) {
      rxSim.update();
      update(txData, rxSim);
      txData.setAvailable(now);
      simNext = now + SIM_INTERVAL_MS * 1000;
    }
  }

  if (c.rxType == MODE_PPM) {
    if (rxPpm.available()) {
      update(txData, rxPpm);
      txData.setAvailable(now);
    }
  }

#ifdef USE_ANALOG
  static uint32_t analogNext = 0;
  if (c.rxType == MODE_ANALOG) {
    if (now > analogNext) {
      rxAnalog.update();
      update(txData, rxAnalog);
      txData.setAvailable(now);
      analogNext = now + 20000;
    }
  }
#endif

  if (c.rxType == MODE_ESPNOW) {
    rxEspNow.update();
    if (rxEspNow.available()) {
      for (size_t channel = 0; channel < 8; ++channel) {
        txData.setChannel(channel, rxEspNow.getChannel(channel));
      }
      txData.setAvailable(now);
    }
  }

  // transmit outputs
 
  static uint32_t sendTimeout = 0;
  static uint32_t sendAfter = 0;
  if (c.txType == MODE_ESPNOW) {
    if ((now > sendAfter && txData.getAvailable()) || now > sendTimeout) {
      for (size_t channel = 0; channel < 8; channel++) {
        txEspNow.setChannel(channel, txData.getChannel(channel));
      }
      sendTimeout = now + 50000ul;
      sendAfter = now + 10000ul;
      txEspNow.commit();
    }
    txEspNow.update();
  }

#ifdef USE_GAMEPAD
  if (c.gamepad) {
    if (txData.getAvailable()) {
      Joystick.setXAxis(gamepadScale(txData.getChannel(0)));
      Joystick.setYAxis(gamepadScale(txData.getChannel(1)));
      Joystick.setThrottle(gamepadScale(txData.getChannel(2)));
      Joystick.setRudder(gamepadScale(txData.getChannel(3)));
      Joystick.setZAxis(gamepadScale(txData.getChannel(4)));
      // channel 6, 7, 8 as buttons
      for (size_t i = 0; i < 3; i++) {
        Joystick.setButton(i, txData.getChannel(i + 5) >= BUTTON_THRESHOLD);
      }
      Joystick.sendState();
    }
  }
#endif

  wireless.update();

  txData.clearAvailable();

  // print debug info
  static uint32_t printNext = 0;
  if (c.debug) {
    if (now > printNext) {
      Serial.printf("%d,%d,%d,%d,%d\n", txData.getChannel(0),
                    txData.getChannel(1), txData.getChannel(2),
                    txData.getChannel(3), txData.getChannel(4));
      if (txData.isCalibrationEnabled()) {
        for (size_t i = 0; i < 4; i++) {
          auto [l, m, h] = txData.getCalibration(i);
          Serial.printf("calibration %d,%d,%d,%d\n", i, l, m, h);
        }
      }
      printNext = now + 200000ul;
    }
  }
}
