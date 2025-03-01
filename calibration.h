#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>

#define RSSI_STABILISATION_TIME 30

void calibrateRssi(int calibratedRssi[2], int index);

#endif
