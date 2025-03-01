#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
public:
  Buzzer(int pin);
  void buzz(int durationMs);
private:
  uint8_t buzzerPin;
};

#endif
