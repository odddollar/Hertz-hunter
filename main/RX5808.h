#ifndef RX5808_H
#define RX5808_H

#include <Arduino.h>

#define RSSI_STABILISATION_TIME 30

class RX5808 {
public:
  RX5808(uint8_t data, uint8_t le, uint8_t clk, uint8_t rssi);
  void scan(int scannedValues[61], int numScannedValues, int minFreq, int interval, SemaphoreHandle_t mutex);
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
