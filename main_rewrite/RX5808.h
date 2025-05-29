#ifndef RX5808_H
#define RX5808_H

#include <Arduino.h>

#define MAX_FREQUENCIES_SCANNED 60 + 1

#define RSSI_STABILISATION_TIME 30
#define RSSI_SAMPLES 30

// RX5808 receiver module
class RX5808 {
public:
  RX5808(uint8_t data, uint8_t le, uint8_t clk, uint8_t rssi);
  void setFrequency(int frequency);
  int readRSSI();
private:
  void reset();
  void sendRegister(byte address, unsigned long data);
  void sendBit(bool bit);
  unsigned long frequencyToRegister(int frequency);
  uint8_t dataPin;
  uint8_t lePin;
  uint8_t clkPin;
  uint8_t rssiPin;
};

#endif
