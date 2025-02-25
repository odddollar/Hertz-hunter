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

  // Setup rssi pin
  pinMode(rssiPin, INPUT);

  // Set inital pin state
  digitalWrite(spiLePin, HIGH);
  digitalWrite(spiClkPin, LOW);

  // Reset module
  reset();
}

// Set module frequency
void RX5808::setFrequency(int frequency) {
  // Track current frequency
  currentFrequency = frequency;

  // Calculate frequency value to send to module
  unsigned long toSend = frequencyToRegister(frequency);

  // Begin transmission
  digitalWrite(spiLePin, LOW);

  // Send address 0x1 (LSB)
  sendBit(1);
  sendBit(0);
  sendBit(0);
  sendBit(0);

  // Set to write mode
  sendBit(1);

  // Send data bits (LSB)
  for (int i = 0; i < 20; i++) {
    sendBit(bitRead(toSend, i));
  }

  // End transmission
  digitalWrite(spiLePin, HIGH);
}

// Read rssi from module
int RX5808::readRSSI() {
  // Record 10 rssi values and average
  int rssi = 0;
  for (int i = 0; i < 10; i++) {
    rssi += analogRead(rssiPin);
  }
  rssi /= 10;

  return rssi;
}

// Reset module
void RX5808::reset() {
  // Begin transmission
  digitalWrite(spiLePin, LOW);

  // Send address 0xF (LSB)
  sendBit(1);
  sendBit(1);
  sendBit(1);
  sendBit(1);

  // Set to write mode
  sendBit(1);

  // Write blank data
  for (int i = 0; i < 20; i++) {
    sendBit(0);
  }

  // End transmission
  digitalWrite(spiLePin, HIGH);
}

// Send 0 or 1 to module
void RX5808::sendBit(bool bit) {
  // Set data value
  digitalWrite(spiDataPin, bit);

  // Pulse clock
  digitalWrite(spiClkPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(spiClkPin, LOW);
}

unsigned long RX5808::frequencyToRegister(int frequency) {
  // Calculate parts to send to module
  frequency -= 479;
  int n = frequency / 64;
  int a = (frequency % 64) / 2;

  // Calculate frequency value to send to module
  return (n << 7) | a;
}
