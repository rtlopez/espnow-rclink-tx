#include <Arduino.h>
#include "EspNowRcLink/Transmitter.h"
#include "ppm.h"

// uncomment to activate simultor on channel 3
//#define SIM_TX_INTERVAL_MS (20 * 1000) // 20ms => 50Hz

// uncomment to use ppm on pin 13
#define PPM_PIN 13

// uncomment to print some details to console
//#define PRINT_INFO

EspNowRcLink::Transmitter tx;
#ifdef PPM_PIN
PPM ppm;
#endif

void setup()
{
  Serial.begin(115200);

#ifdef PPM_PIN
  ppm.begin(PPM_PIN, FALLING);
#endif

  tx.begin(true);
}

void loop()
{
  uint32_t now = micros();
  static int v = 0;
  static uint32_t delta = 0;

#ifdef PPM_PIN
  static uint32_t lastSent = 0;
  uint32_t sentPeriod = (now - lastSent);
  if(ppm.available() || sentPeriod >= 50000ul)
  {
    lastSent = now;
    delta = sentPeriod;
    for(size_t c = 0; c < 8; c++)
    {
      const int16_t val = ppm.get(c);
      tx.setChannel(c, val);
      if(c == 2) v = val;
    }
    tx.commit();
    //Serial.println(v);
  }
#endif

#ifdef SIM_TX_INTERVAL_MS
  static int sim_val = 0;
  static bool sim_dir = true;
  const int sim_rate = 4;

  static uint32_t sendNext = now + SIM_TX_INTERVAL_MS;

  // send rc channels
  if(now >= sendNext)
  {
    v = 1000 + sim_val;
    tx.setChannel(2, v);
    tx.commit();
    sendNext = now + TX_INTERVAL_MS;

    if(sim_dir)
    {
      sim_val += sim_rate;
      if(sim_val >= 1000) sim_dir = false;
    }
    else
    {
      sim_val -= sim_rate;
      if(sim_val <= 0) sim_dir = true;
    }
  }
#endif
  
  tx.update();

#ifdef PRINT_INFO
  static uint32_t printNext = now + 500000;
  if(now >= printNext)
  {
    Serial.printf("V: %d, P: %d, D: %d, C: %d\n", v, ppm.get(2), delta / 100, WiFi.channel());
    printNext = now + 500000;
  }
#else
  (void)v;
  (void)delta;
#endif
}