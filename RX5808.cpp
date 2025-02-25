#include "RX5808.h"

// Initialise RX5808 module
RX5808::RX5808(uint8_t dataPin, uint8_t lePin, uint8_t clkPin, uint8_t rssiPin) {
  spiDataPin = dataPin;
  spiLePin = lePin;
  spiClkPin = clkPin;
  rssiPin = rssiPin;

  // Setup spi pins
  pinMode(spiDataPin, OUTPUT);
  pinMode(spiLePin, OUTPUT);
  pinMode(spiClkPin, OUTPUT);

  // Set inital pin state
  digitalWrite(spiLePin, HIGH);
  digitalWrite(spiClkPin, LOW);
}

// Set module frequency
void RX5808::setFrequency(int frequency) {
  // Calculate parts to send to module
  int freq = frequency - 479;
  int n = freq / 64;
  int a = (freq % 64) / 2;

  // Calculate frequency value to send to module
  unsigned long toSend = (n << 7) | a;
}

// Read rssi from module
int RX5808::readRSSI() {
  int freq = 5800 - 479;

  // N part
  int n = freq / 64;

  // A part
  int a = (freq % 64) / 2;

  return (n << 7) | a;
}

// Send 0 or 1 to module
void RX5808::sendBit(int bit) {
  // Set data value
  digitalWrite(spiDataPin, bit);

  // Pulse clock
  digitalWrite(spiClkPin, HIGH);
  digitalWrite(spiClkPin, LOW);
}
