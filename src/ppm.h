#pragma once

#include <Arduino.h>

class PPM
{
public:
    PPM();
    int begin(int pin, int mode = RISING);
    void end();
    bool available();
    int16_t get(int channel);

private:
    void handle();
    static void handle_isr(void* arg);
    static const size_t CHANNELS = 16;

    int _pin = -1;
    volatile size_t _channel = 0;
    volatile uint32_t _last_tick = 0;
    volatile bool _new_data = false;
    volatile uint32_t _channels[CHANNELS];
};
