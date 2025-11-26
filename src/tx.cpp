#include "tx.hpp"
#include <EEPROM.h>

Tx::Tx() {
  std::fill_n(_values, CHANNEL_NUM, 1500);
  _values[2] = 1000;
}

int Tx::begin() {
  EEPROM.begin(256);
  return load();
}

void Tx::setChannel(size_t channel, int value) { _values[channel] = value; }

int Tx::getChannel(size_t channel) const { return _values[channel]; }

bool Tx::getAvailable() const { return _available; }

void Tx::setAvailable(uint32_t timestampUs) {
  _available = true;
  _deltaTimeUs = timestampUs - _lastUpdateTimeUs;
  _lastUpdateTimeUs = timestampUs;
}

void Tx::clearAvailable() { _available = false; }

void Tx::setFailSafe() {
  if (!_failsafe) {
    std::fill_n(_values, CHANNEL_NUM, 1500);
    _values[2] = 1000;
    _failsafe = true;
    dispatch(EV_FAILSAFE_ENTER);
  }
}

void Tx::clearFailSafe() {
  if (_failsafe) {
    _failsafe = false;
    dispatch(EV_FAILSAFE_EXIT);
  }
}

size_t Tx::getChannelCount() const { return CHANNEL_NUM; }

uint32_t Tx::getDeltaTime() const { return _deltaTimeUs; }

std::tuple<int, int, int> Tx::getCalibration(size_t channel) const {
  if (channel < 4) {
    auto &cal = _calibration[channel];
    return {cal.inMin, cal.inMid, cal.inMax};
  }
  return {0, 0, 0};
}

void Tx::setCalibration(size_t channel, int16_t inMin, int16_t inMid,
                        int16_t inMax) {
  if (channel < 4) {
    auto &cal = _calibration[channel];
    cal.inMin = inMin;
    cal.inMid = inMid;
    cal.inMax = inMax;
  }
}

void Tx::enableCalibration() {
  _calibrationActive = true;
  for (auto &in : _calibration) {
    in.inMin = 2000;
    in.inMax = 2100;
  }
}

bool Tx::isCalibrationEnabled() const { return _calibrationActive; }

bool Tx::isCalibrationDone() const {
  for (size_t i = 0; i < 4; ++i) {
    auto &c = _calibration[i];
    if (c.inMax - c.inMin < 1000)
      return false;
  }
  return true;
}

TxConfig &Tx::getConfig() { return _config; }

void Tx::save() {
  _calibrationActive = false;
  size_t addr = 0;
  EEPROM.write(addr++, 0xAA);                                   // header 1
  EEPROM.write(addr++, 0x55);                                   // header 2
  EEPROM.write(addr++, 0x00);                                   // version
  EEPROM.write(addr++, sizeof(_calibration) + sizeof(_config)); // size
  EEPROM.put(addr, _calibration);                               // payload
  addr += sizeof(_calibration);
  EEPROM.put(addr, _config); // payload
  EEPROM.commit();
  dispatch(EV_SAVE);
}

int Tx::load() {
  size_t addr = 0;
  if (EEPROM.read(addr++) != 0xAA) {
    return 1; // header 1
  }
  if (EEPROM.read(addr++) != 0x55) {
    return 2; // header 2
  }
  if (EEPROM.read(addr++) != 0x00) {
    return 3; // version
  }
  if (EEPROM.read(addr++) != sizeof(_calibration) + sizeof(_config)) {
    return 4; // size
  }
  EEPROM.get(addr, _calibration); // payload
  addr += sizeof(_calibration);
  EEPROM.get(addr, _config); // payload
  return 0;
}