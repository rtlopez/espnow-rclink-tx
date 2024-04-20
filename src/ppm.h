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
    void IRAM_ATTR handle();
    static void IRAM_ATTR handle_isr();

    static const size_t CHANNELS = 16;
    static PPM* _instance;

    int _pin = -1;
    volatile size_t _channel = 0;
    volatile uint32_t _last_tick = 0;
    volatile bool _new_data = false;
    volatile uint32_t _channels[CHANNELS];
};
