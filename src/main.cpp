#include <Arduino.h>
#include "tx.hpp"

#if defined(PIN_PPM) && defined(USE_ANALOG)
#error "PIN_PPM and USE_ANALOG Cannot be defined at the same time!"
#endif

#define SIM_INTERVAL_MS 20 // 20ms => 50Hz

#ifdef MODE_SIM
#include "sim.hpp"
Sim rxSim;
#endif

#ifdef PIN_PPM
#include "ppm.h"
PPM rxPpm;
#endif

#ifdef USE_ANALOG
#include "analog.hpp"
Analog rxAnalog;
#endif

#ifdef MODE_RX
#include "EspNowRcLink/Receiver.h"
EspNowRcLink::Receiver rxEspNow;
#endif

#ifdef MODE_TX
#include "EspNowRcLink/Transmitter.h"
EspNowRcLink::Transmitter txEspNow;
#endif

#ifdef MODE_GAMEPAD
#include <Joystick_ESP32S2.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 3, 0,
  true , true , true,      // axis x,y,z
  false, false, false,     // rotation x,y,z
  true , true,             // throttle, rudder
  false, false, false      // accel, brake, steering
);

#define INPUT_OFFSET -1500
#define INPUT_RANGE_MIN -512
#define INPUT_RANGE_MAX 512
#define AXIS_RANGE_MIN -512
#define AXIS_RANGE_MAX 512
#define BUTTON_THRESHOLD 1650

int16_t gamepadScale(int16_t val)
{
  return constrain(val + INPUT_OFFSET, INPUT_RANGE_MIN, INPUT_RANGE_MAX);
}
#endif

Tx txData;

void setup()
{
#ifdef PRINT_INFO
  Serial.begin(115200);
#endif

#ifdef MODE_SIM
  rxSim.begin();
#else
#ifdef PIN_PPM
  rxPpm.begin(PIN_PPM, FALLING);
#endif

#ifdef USE_ANALOG
  rxAnalog.attach(0, Analog::MODE_ANALOG, PIN_POT0);
  rxAnalog.attach(1, Analog::MODE_ANALOG, PIN_POT1);
  rxAnalog.attach(2, Analog::MODE_ANALOG, PIN_POT2);
  rxAnalog.attach(3, Analog::MODE_ANALOG, PIN_POT3);
  rxAnalog.attach(4, Analog::MODE_DIGITAL, PIN_SW0);
  rxAnalog.attach(5, Analog::MODE_DIGITAL, PIN_SW1);
  rxAnalog.attach(6, Analog::MODE_DIGITAL, PIN_SW2);
  rxAnalog.attach(7, Analog::MODE_DIGITAL, PIN_SW3);
#endif

#ifdef MODE_RX
  rxEspNow.begin(true);
#endif
#endif

#ifdef MODE_TX
  txEspNow.begin(true);
#endif

#ifdef MODE_GAMEPAD
  Joystick.setXAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
  Joystick.setYAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
  Joystick.setZAxisRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
  Joystick.setThrottleRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
  Joystick.setRudderRange(AXIS_RANGE_MIN, AXIS_RANGE_MAX);
  Joystick.begin(false);
#endif
}

template<typename Dst, typename Src>
void update(Dst& dst, const Src& src)
{
  for(size_t channel = 0; channel < 8; ++channel)
  {
    dst.setChannel(channel, src.get(channel));
  }
}

void loop()
{
  uint32_t now = micros();

  // receive inputs

#ifdef MODE_SIM
  static uint32_t simNext = 0;
  if(now > simNext)
  {
    rxSim.update();
    update(txData, rxSim);
    txData.setAvailable(now);
    simNext = now + SIM_INTERVAL_MS * 1000;
  }
#else
#ifdef PIN_PPM
  if(rxPpm.available())
  {
    update(txData, rxPpm);
    txData.setAvailable(now);
  }
#endif

#ifdef USE_ANALOG
  static uint32_t analogNext = 0;
  if(now > analogNext)
  {
    rxAnalog.update();
    update(txData, rxAnalog);
    txData.setAvailable(now);
    analogNext = now + 20000;
  }
#endif

#ifdef MODE_RX
  rxEspNow.update();
  if(rxEspNow.available())
  {
    for(size_t channel = 0; channel < 8; ++channel)
    {
      txData.setChannel(channel, rxEspNow.getChannel(channel));
    }
    txData.setAvailable(now);
  }
#endif
#endif

  // transmit outputs

#ifdef MODE_TX

  static uint32_t sendTimeout = 0;
  static uint32_t sendAfter = 0;
  if((now > sendAfter && txData.getAvailable()) || now > sendTimeout)
  {
    for(size_t channel = 0; channel < 8; channel++)
    {
      txEspNow.setChannel(channel, txData.getChannel(channel));
    }
    sendTimeout = now + 50000ul;
    sendAfter = now + 10000ul;
    txEspNow.commit();
  }
  txEspNow.update();
#endif

#ifdef MODE_GAMEPAD
  if(txData.getAvailable())
  {
    Joystick.setXAxis(gamepadScale(txData.getChannel(0)));
    Joystick.setYAxis(gamepadScale(txData.getChannel(1)));
    Joystick.setThrottle(gamepadScale(txData.getChannel(2)));
    Joystick.setRudder(gamepadScale(txData.getChannel(3)));
    Joystick.setZAxis(gamepadScale(txData.getChannel(4)));
    // channel 6, 7, 8 as buttons
    for(size_t i = 0; i < 3; i++)
    {
      Joystick.setButton(i, txData.getChannel(i + 5) >= BUTTON_THRESHOLD);
    }
    Joystick.sendState();
  }
#endif

#ifdef PRINT_INFO
  // print debug info
  static uint32_t printNext = 0;
  if(now > printNext)
  {
    Serial.printf("%d,%d,%d,%d,%d\n",
      txData.getChannel(0), txData.getChannel(1), txData.getChannel(2), txData.getChannel(3), txData.getChannel(4)
    );
    printNext = now + 100000ul;
  }
#endif
}
