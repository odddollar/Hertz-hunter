#ifndef RX5808_H
#define RX5808_H

#include <Arduino.h>

class RX5808 {
public:
  RX5808(uint8_t dataPin, uint8_t lePin, uint8_t clkPin, uint8_t rssiPin);
  void setFrequency(int frequency);
  int readRSSI();
private:
  void reset();
  void sendBit(bool bit);
  unsigned long frequencyToRegister(int frequency);
  uint8_t spiDataPin;
  uint8_t spiLePin;
  uint8_t spiClkPin;
  uint8_t rssiPin;
  int currentFrequency;
};

#endif
