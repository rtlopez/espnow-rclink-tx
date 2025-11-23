#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>

struct TxCalibrationData {
  int16_t inMin = 0;
  int16_t inMid = 2047;
  int16_t inMax = 4095;
};

enum RxTxMode: uint8_t {
  MODE_NONE = 0,
  MODE_ESPNOW,
  MODE_SIM,
  MODE_ANALOG,
  MODE_PPM,
  MODE_SBUS,
  MODE_IBUS,
  MODE_CRSF,
};

struct TxConfig {
  RxTxMode rxType = MODE_NONE;
  RxTxMode txType = MODE_NONE;
  int8_t rxPin = -1;
  int8_t txPin = -1;
  int8_t analogPins[4];
  int8_t buttonPins[4];
  int8_t gamepad = 0;
  int8_t debug = 0;
};

class Tx {
public:
  Tx();

  int begin();

  void setChannel(size_t channel, int value);
  int getChannel(size_t channel) const;

  void setAvailable(uint32_t timestampUs);
  void clearAvailable();
  bool getAvailable() const;

  size_t getChannelCount() const;
  uint32_t getDeltaTime() const;

  std::tuple<int, int, int> getCalibration(size_t channel) const;
  void setCalibration(size_t channel, int16_t inMin, int16_t inMid, int16_t inMax);

  void enableCalibration();
  bool isCalibrationEnabled() const;
  bool isCalibrationDone() const;

  TxConfig& getConfig();

  void save();
  int load();

private:
  static constexpr size_t CHANNEL_NUM = 16;
  int _values[CHANNEL_NUM];
  bool _available = false;
  uint32_t _lastUpdateTimeUs = 0;
  uint32_t _deltaTimeUs = 0;
  bool _calibrationActive = false;

  TxConfig _config;
  TxCalibrationData _calibration[4];
};
