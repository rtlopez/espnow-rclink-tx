#pragma once

#include <cstddef>
#include <algorithm>

class Tx
{
public:
  Tx()
  {
    std::fill_n(_values, CHANNEL_NUM, 1500);
    _values[2] = 1000;
  }

  void setChannel(size_t channel, int value)
  {
    _values[channel] = value;
  }

  int getChannel(size_t channel) const
  {
    return _values[channel];
  }

  void setAvailable(uint32_t timestampUs)
  {
    _available = true;
    _deltaTimeUs = timestampUs - _lastUpdateTimeUs;
    _lastUpdateTimeUs = timestampUs;
  }

  bool getAvailable()
  {
    auto ret = _available;
    _available = false;
    return ret;
  }

  size_t getChannelCount() const
  {
    return CHANNEL_NUM;
  }

  uint32_t getDeltaTime() const
  {
    return _deltaTimeUs;
  }

private:
  static constexpr size_t CHANNEL_NUM = 16;
  int _values[CHANNEL_NUM];
  bool _available = false;
  uint32_t _lastUpdateTimeUs = 0;
  uint32_t _deltaTimeUs = 0;
};
