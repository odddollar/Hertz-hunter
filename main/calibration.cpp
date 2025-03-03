#include "calibration.h"
#include "module.h"

// Set to listen to specific channel and assign rssi values to high or low calibration
void calibrateRssi(int calibratedRssi[2], int index) {
  // Set module frequency to F4
  module.setFrequency(5800);

  // Give time for rssi to stabilise
  delay(RSSI_STABILISATION_TIME);

  // Index 0 is high but needs to be low
  index = (index + 1) % 2;

  // Read rssi into array
  calibratedRssi[index] = module.readRSSI();
}
