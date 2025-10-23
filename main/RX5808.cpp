#include "RX5808.h"

// Initialise RX5808 module
RX5808::RX5808(uint8_t data, uint8_t le, uint8_t clk, uint8_t rssi, Settings *s)
  : rssiValues(0), lowband(false),
    dataPin(data), lePin(le), clkPin(clk), rssiPin(rssi),
    scanHandle(NULL), settings(s) {

  // Setup spi pins
  pinMode(dataPin, OUTPUT);
  pinMode(lePin, OUTPUT);
  pinMode(clkPin, OUTPUT);

  // Setup rssi pin
  pinMode(rssiPin, INPUT);

  // Set inital pin state
  digitalWrite(lePin, HIGH);
  digitalWrite(clkPin, LOW);

  // Create scanning mutex
  scanMutex = xSemaphoreCreateMutex();

  // Reset module
  reset();
}

// Start background scanning
void RX5808::startScan() {
  // Start scanning task only if not already running
  if (scanHandle == NULL) {
    xTaskCreate(_scan, "scan", SCAN_STACK_SIZE, this, 1, &scanHandle);
  }
}

// Stop background scanning
void RX5808::stopScan() {
  // Cancel scanning task only if already running
  if (scanHandle != NULL) {
    vTaskDelete(scanHandle);
    scanHandle = NULL;
  }
}

// Save current rssi as high/low calibration
void RX5808::calibrate(bool high) {
  // Set to F4
  setFrequency(5800);

  // Give time for rssi to stabilise
  delay(RSSI_STABILISATION_TIME);

  // Save rssi
  if (high) {
    settings->highCalibratedRssi.set(readRSSI());
  } else {
    settings->lowCalibratedRssi.set(readRSSI());
  }
}

// Background task that runs scanning continuously
void RX5808::_scan(void *parameter) {
  // Static cast weirdness to access parameters
  RX5808 *module = static_cast<RX5808 *>(parameter);

  // Get interval at which to scan
  float interval = module->settings->scanInterval.get();

  // Calculate number of values to scan
  int numScannedValues = (SCAN_FREQUENCY_RANGE / interval) + 1;  // +1 for final number inclusion

  // Loop continuously
  // Stops when scanning task cancelled
  while (1) {
    for (int i = 0; i < numScannedValues; i++) {
      // Get minimum frequency to support changing to lowband
      int min_freq = module->lowband.get() ? LOWBAND_MIN_FREQUENCY : HIGHBAND_MIN_FREQUENCY;

      // Set frequency and offset by minimum
      module->setFrequency((int)round(i * interval + min_freq));

      // Give time for rssi to stabilise
      vTaskDelay(pdMS_TO_TICKS(RSSI_STABILISATION_TIME));

      // Take mutex to safely modify data in this task
      if (xSemaphoreTake(module->scanMutex, portMAX_DELAY)) {
        module->rssiValues.set(i, module->readRSSI());

        xSemaphoreGive(module->scanMutex);
      }
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
  // Record multiple rssi values and average
  int rssi = 0;
  for (int i = 0; i < RSSI_SAMPLES; i++) {
    rssi += analogRead(rssiPin);
  }
  rssi /= RSSI_SAMPLES;

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
