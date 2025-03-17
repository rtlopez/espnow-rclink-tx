#pragma once

#include <cstddef>

class Sim
{
public:
  int get(size_t channel)
  {
    if(_dir)
    {
      _val += _rate;
      if(_val >= 1000) _dir = false;
    }
    else
    {
      _val -= _rate;
      if(_val <= 0) _dir = true;
    }
    return _val;
  }
private:
  bool _dir = true;
  const int _rate = 4;
  int _val;
};

