#ifndef RX5808_H
#define RX5808_H

#include <Arduino.h>
#include "settings.h"
#include "variable.h"

#define MAX_FREQUENCIES_SCANNED 120 + 1
#define HIGHBAND_MIN_FREQUENCY 5645
#define LOWBAND_MIN_FREQUENCY 5345
#define SCAN_FREQUENCY_RANGE 300

#define RSSI_STABILISATION_TIME 30
#define RSSI_SAMPLES 30

#define SCAN_STACK_SIZE 2048

// RX5808 receiver module
class RX5808 {
public:
  RX5808(uint8_t data, uint8_t le, uint8_t clk, uint8_t rssi, Settings *s);
  void startScan();
  void stopScan();
  void calibrate(bool high);

  VariableArrayRestricted<int, MAX_FREQUENCIES_SCANNED> rssiValues;
  Variable<bool> lowband;

  SemaphoreHandle_t scanMutex;

private:
  static void _scan(void *parameter);
  void setFrequency(int frequency);
  int readRSSI();
  void reset();
  void sendRegister(byte address, unsigned long data);
  void sendBit(bool bit);
  unsigned long frequencyToRegister(int frequency);

  uint8_t dataPin;
  uint8_t lePin;
  uint8_t clkPin;
  uint8_t rssiPin;

  TaskHandle_t scanHandle;

  Settings *settings;
};

#endif
