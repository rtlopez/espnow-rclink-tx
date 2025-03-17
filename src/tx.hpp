#pragma once

#include <cstddef>
#include <algorithm>

class Tx
{
public:
  Tx()
  {
    std::fill_n(_values, CHANNEL_NUM, 0);
  }

  void setChannel(size_t channel, int value)
  {
    _values[channel] = value;
  }

  int getChannel(size_t channel) const
  {
    return _values[channel];
  }

  void setAvailable()
  {
    _available = true;
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

private:
  static constexpr size_t CHANNEL_NUM = 16;
  int _values[CHANNEL_NUM];
  bool _available = false;
};
