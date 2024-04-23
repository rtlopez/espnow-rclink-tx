# EspNow RCLink Transmitter

Designed for use in small short-range remote-controlled aircraft, vehicles and [ESP-FC](https://github.com/rtlopez/esp-fc) flight controller software.

Simple implementation of external module with EspNow RcLink protocol. Just upload code to ESP32 module and connect to your transmitters JR bay.

Module accepts `PPM` signal on `GPIO13` and transmits 8 RC channels.

## Connection diagram

![EspNow RcLink connections](/docs/img/espnow_rclink_tx_bb.png)

> [!NOTE]
> Keep eye on power consumption, linear voltage regulator might get hot. Switching regulator (step-down) is recommended if voltage drop is high.

## Flashing

1. Download and unzip selected firmware from [Releases Page](https://github.com/rtlopez/espnow-rclink-tx/releases)
2. Visit [ESP Tool Website](https://espressif.github.io/esptool-js/)
3. Click "Connect" and choose device port in dialog
4. Add firmware file and set Flash Address to `0x00`
5. Click "Program"
6. After success power cycle board

![EspNow RcLink Flashing](/docs/img/esptool-js-flash-connect.png)

Your module is redy to use.

## Using with ESP-FC

To use [ESP-FC](https://github.com/rtlopez/esp-fc) as receiver just select `SPI Rx (e.g. built-in Rx)` as Receiver mode. Receiver provider doesn't matter here.

![ESP-FC receiver](/docs/img/espfc_receiver.png)

Transmitter and receiver binds automatically after power up, you don't need to do anything. Recomended startup procedure is:
  1. turn on transmitter first
  2. next power up receiver/flight controller
