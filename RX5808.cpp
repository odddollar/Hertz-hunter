#include "RX5808.h"

// Initialise RX5808 module
RX5808::RX5808(uint8_t data, uint8_t le, uint8_t clk, uint8_t rssi) {
  dataPin = data;
  lePin = le;
  clkPin = clk;
  rssiPin = rssi;

  // Setup spi pins
  pinMode(dataPin, OUTPUT);
  pinMode(lePin, OUTPUT);
  pinMode(clkPin, OUTPUT);

  // Setup rssi pin
  pinMode(rssiPin, INPUT);

  // Set inital pin state
  digitalWrite(lePin, HIGH);
  digitalWrite(clkPin, LOW);

  // Reset module
  reset();
}

// Scan frequency range at set interval
void RX5808::scan(int scannedValues[60], int numScannedValues, int minFreq, int interval) {
  // Iterate through frequencies to scan
  for (int i = 0; i <= numScannedValues; i++) {
    // Set frequency and offset by minimum
    setFrequency(i * interval + minFreq);

    // Give time for rssi to stabilise
    delay(RSSI_STABILISATION_TIME);

    // Store rssi in corresponding index
    scannedValues[i] = readRSSI();
  }
}

// Set module frequency
void RX5808::setFrequency(int frequency) {
  // Track current frequency
  currentFrequency = frequency;

  // Calculate frequency value to send to module
  unsigned long toSend = frequencyToRegister(frequency);

  // Begin transmission
  digitalWrite(lePin, LOW);

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
  digitalWrite(lePin, HIGH);
}

// Read rssi from module
int RX5808::readRSSI() {
  // Record 10 rssi values and average
  int rssi = 0;
  for (int i = 0; i < 30; i++) {
    rssi += analogRead(rssiPin);
  }
  rssi /= 30;

  return rssi;
}

// Reset module
void RX5808::reset() {
  // Begin transmission
  digitalWrite(lePin, LOW);

  // Send address 0xF (LSB)
  sendBit(1);
  sendBit(1);
  sendBit(1);
  sendBit(1);

  // Set to write mode
  sendBit(1);

  // Write blank data (LSB)
  for (int i = 0; i < 20; i++) {
    sendBit(0);
  }

  // End transmission
  digitalWrite(lePin, HIGH);
}

// Send 0 or 1 to module
void RX5808::sendBit(bool bit) {
  // Set data value
  digitalWrite(dataPin, bit);

  // Pulse clock
  digitalWrite(clkPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(clkPin, LOW);
}

unsigned long RX5808::frequencyToRegister(int frequency) {
  // Calculate parts to send to module
  frequency -= 479;
  int n = frequency / 64;
  int a = (frequency % 64) / 2;

  // Calculate frequency value to send to module
  return (n << 7) | a;
}
