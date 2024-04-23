#include "ppm.h"

PPM::PPM()
{
  for(size_t i = 0; i < CHANNELS; i++)
  {
    _channels[i] = i == 2 ? 1000 : 1500;
  }
}

void PPM::end()
{
  if(_pin != -1)
  {
    detachInterrupt(_pin);
    _pin = -1;
  }
}

int PPM::begin(int pin, int mode)
{
  end();
  if(pin == -1) return 0;

  _pin = pin;
  _channel = 0;
  _last_tick = micros();

  pinMode(_pin, INPUT);
  attachInterruptArg(_pin, &PPM::handle_isr, this, mode);

  return 1;
}

bool PPM::available()
{
  if(_new_data)
  {
    _new_data = false;
    return true;
  }
  return false;
}

int16_t PPM::get(int channel)
{
  return _channels[channel];
}

void IRAM_ATTR PPM::handle_isr(void* arg)
{
  if(arg) reinterpret_cast<PPM*>(arg)->handle();
}

void IRAM_ATTR PPM::handle()
{
  uint32_t now = micros();
  uint32_t width = now - _last_tick;

  _last_tick = now;

  if(width > 2500ul) // sync
  {
    _channel = 0;
    _new_data = true;
    return;
  }

  if(_channel < CHANNELS) // ignore exceding channels
  {
    _channels[_channel] = width;
  }
  _channel++;
}
