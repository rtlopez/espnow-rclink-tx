#include <Arduino.h>
#include "EspNowRcLink/Transmitter.h"
#include "tx.hpp"
#include "sim.hpp"

#if defined(PIN_PPM) && defined(USE_ANALOG)
#error "PIN_PPM and USE_ANALOG Cannot be defined at the same time!"
#endif

#ifdef PIN_PPM
#include "ppm.h"
PPM rxPpm;
#endif

#ifdef USE_ANALOG
#include "analog.hpp"
Analog rxAnalog;
#endif

// uncomment to activate simultor on channel 3
// #define SIM_INTERVAL_MS 20 // 20ms => 50Hz

Sim rxSim;
Tx txData;
EspNowRcLink::Transmitter txEspNow;

void setup()
{
#ifdef PRINT_INFO
  Serial.begin(115200);
#endif

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
#endif

  txEspNow.begin(true);
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
  uint32_t rxDelta = 0;

  // decode inputs

#ifdef SIM_INTERVAL_MS
  static uint32_t simNext = 0;
  if(now >= simNext)
  {
    txData.setChannel(2, 1000 + rxSim.get(2));
    txData.setAvailable();
    simNext = now + SIM_INTERVAL_MS * 1000;
  }
#else

#ifdef PIN_PPM
  if(rxPpm.available())
  {
    update(txData, rxPpm);
    txData.setAvailable();
  }
#endif

#ifdef USE_ANALOG
  static uint32_t analogNext = 0;
  if(now > analogNext)
  {
    rxAnalog.update();
    update(txData, rxAnalog);
    txData.setAvailable();
    analogNext = now + 20000;
  }
#endif

#endif

  // transmit outputs

  static uint32_t sendTimeout = 0;
  static uint32_t sendAfter = 0;
  static uint32_t sendLast = 0;
  if((now > sendAfter && txData.getAvailable()) || now > sendTimeout)
  {
    for(size_t c = 0; c < 8; c++)
    {
      txEspNow.setChannel(c, txData.getChannel(c));
    }
    sendTimeout = now + 50000ul;
    sendAfter = now + 10000ul;
    rxDelta = now - sendLast;
    sendLast = now;

    txEspNow.commit();
  }
  txEspNow.update();

  // print debug info

#ifdef PRINT_INFO
  static uint32_t printNext = 0;
  if(now > printNext)
  {
    //Serial.printf("V: %d, P: %d, D: %d, C: %d\n", v, sim_val, delta / 100, WiFi.channel());
    Serial.printf("%d,%d,%d,%d,%d\n",
      txData.getChannel(0), txData.getChannel(1), txData.getChannel(2), txData.getChannel(3), txData.getChannel(4)
    );
    printNext = now + 100000ul;
  }
#endif
}
