#include "RX5808.h"

// Initialise RX5808 module
RX5808::RX5808(uint8_t data, uint8_t le, uint8_t clk, uint8_t rssi)
  : dataPin(data), lePin(le), clkPin(clk), rssiPin(rssi) {

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
void RX5808::scan(int scannedValues[61], int numScannedValues, int minFreq, int interval, SemaphoreHandle_t mutex) {
  // Iterate through frequencies to scan
  for (int i = 0; i < numScannedValues; i++) {
    // Set frequency and offset by minimum
    setFrequency(i * interval + minFreq);

    // Give time for rssi to stabilise
    vTaskDelay(pdMS_TO_TICKS(RSSI_STABILISATION_TIME));

    // Take mutex to safely modify in this thread
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      // Store rssi in corresponding index
      scannedValues[i] = readRSSI();

      xSemaphoreGive(mutex);
    }
  }
}

// Set module frequency
void RX5808::setFrequency(int frequency) {
  // Calculate frequency value to send to module
  unsigned long toSend = frequencyToRegister(frequency);

  // Send data to 0x1 register
  sendRegister(0x01, toSend);
}

// Read rssi from module
int RX5808::readRSSI() {
  // Record 30 rssi values and average
  int rssi = 0;
  for (int i = 0; i < 30; i++) {
    rssi += analogRead(rssiPin);
  }
  rssi /= 30;

  return rssi;
}

// Reset module
void RX5808::reset() {
  sendRegister(0x0F, 0b00000000000000000000);
}

// Send data to specified module register
void RX5808::sendRegister(byte address, unsigned long data) {
  // Begin transmission
  digitalWrite(lePin, LOW);

  // Send address (LSB)
  for (int i = 0; i < 4; i++) {
    sendBit(bitRead(address, i));
  }

  // Set to write mode
  sendBit(1);

  // Send data bits (LSB)
  for (int i = 0; i < 20; i++) {
    sendBit(bitRead(data, i));
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

// Convert frequency number to required binary representation
unsigned long RX5808::frequencyToRegister(int frequency) {
  // Calculate parts to send to module
  frequency -= 479;
  int n = frequency / 64;
  int a = (frequency % 64) / 2;

  // Calculate frequency value to send to module
  return (n << 7) | a;
}
