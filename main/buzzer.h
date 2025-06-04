#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

#define BUZZ_DURATION 20
#define BUZZ_DELAY 80

#define BUZZER_STACK_SIZE 512

// Buzzer class for buzzer module
class Buzzer {
public:
  Buzzer(uint8_t p);
  void buzz();
  void doubleBuzz();
  void startAlarm();
  void stopAlarm();

private:
  static void _buzz(void *parameter);
  static void _doubleBuzz(void *parameter);
  static void _alarm(void *parameter);

  uint8_t pin;
  
  TaskHandle_t alarmHandle;
};

#endif
