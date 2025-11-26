#pragma once

#include "event.hpp"
#include <cstddef>
#include <cstdint>
#include <tuple>

struct TxCalibrationData {
  int16_t inMin = 0;
  int16_t inMid = 2047;
  int16_t inMax = 4095;
};

enum RxTxMode : uint8_t {
  MODE_NONE,
  MODE_ESPNOW,
  MODE_ANALOG,
  MODE_PPM,
  MODE_SBUS,
  MODE_IBUS,
  MODE_CRSF,
  MODE_SIM,
};

struct TxConfig {
  RxTxMode rxType = MODE_NONE;
  RxTxMode txType = MODE_NONE;
  int8_t rxPin = -1;
  int8_t txPin = -1;
  int8_t analogPins[4] = {-1, -1, -1, -1};
  int8_t buttonPins[4] = {-1, -1, -1, -1};
  int8_t gamepad = 0;
  int8_t debug = 0;
};

class Tx : public EventEmiter {
public:
  Tx();

  int begin();
  void setChannel(size_t channel, int value);
  int getChannel(size_t channel) const;

  template <typename Rcv> void update(const Rcv &rcv) {
    for (size_t channel = 0; channel < 8; ++channel) {
      setChannel(channel, rcv.get(channel));
    }
  }

  bool getAvailable() const;
  void setAvailable(uint32_t timestampUs);
  void clearAvailable();
  void setFailSafe();
  void clearFailSafe();
  uint32_t getDeltaTime() const;

  size_t getChannelCount() const;

  std::tuple<int, int, int> getCalibration(size_t channel) const;
  void setCalibration(size_t channel, int16_t inMin, int16_t inMid,
                      int16_t inMax);

  void enableCalibration();
  bool isCalibrationEnabled() const;
  bool isCalibrationDone() const;

  TxConfig &getConfig();

  void save();
  int load();

private:
  static constexpr size_t CHANNEL_NUM = 16;
  int _values[CHANNEL_NUM];
  bool _available = false;
  uint32_t _lastUpdateTimeUs = 0;
  uint32_t _deltaTimeUs = 0;
  bool _calibrationActive = false;
  bool _failsafe = false;

  TxConfig _config;
  TxCalibrationData _calibration[4];
};
