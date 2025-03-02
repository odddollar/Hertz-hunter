#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
public:
  Buzzer(int pin);
  void buzz(int buzzMs);
  void doubleBuzz(int buzzMs, int delayMs);
private:
  uint8_t buzzerPin;
};

#endif
