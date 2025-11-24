#include "analog.hpp"
#include "espnow.hpp"
#include "gamepad.hpp"
#include "monitor.hpp"
#include "ppm.hpp"
#include "sim.hpp"
#include "tx.hpp"
#include "wireless.hpp"
#include <Arduino.h>

Tx tx;
Sim rxSim(tx);
PPM rxPpm(tx);
Analog rxAnalog(tx);
EspNowLink espnow(tx);
Wireless wireless(tx, "espnow-rclink-tx");
Gamepad gamepad(tx);
Monitor monitor(tx);

void setup() {
  tx.begin();
  TxConfig &c = tx.getConfig();

  monitor.begin();

  if (c.rxType == MODE_SIM) {
    rxSim.begin();
  }
  if (c.rxType == MODE_PPM) {
    rxPpm.begin(c.rxPin, FALLING);
  }

  if (c.rxType == MODE_ANALOG) {
    rxAnalog.begin();
  }

  espnow.begin();
  gamepad.begin();
  wireless.begin();
}

void loop() {
  TxConfig &c = tx.getConfig();

  if (c.rxType == MODE_SIM) {
    rxSim.update();
  }

  if (c.rxType == MODE_PPM) {
    rxPpm.update();
  }

  if (c.rxType == MODE_ANALOG) {
    rxAnalog.update();
  }

  espnow.update();
  wireless.update();

  uint32_t now = micros();
  static uint32_t lastRecv = now;
  static uint32_t lastSent = now;
  static bool failSafe = false;
  if (tx.getAvailable()) {
    tx.clearFailSafe();
    tx.dispatch(EV_CHANNEL_UPDATE);
    tx.clearAvailable();
    lastRecv = now;
    lastSent = now;
  }
  // activate failsafe if no pulses for 150ms
  if (now > lastRecv + 150000) {
    tx.setFailSafe();
  }
  // ensure minimal interval, repeat every 20ms
  if (now > lastSent + 20000) {
    tx.dispatch(EV_CHANNEL_UPDATE);
    lastSent = now;
  }
}
