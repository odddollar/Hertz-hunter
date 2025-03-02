#include "buzzer.h"

// Initialise buzzer module
Buzzer::Buzzer(int pin) {
  buzzerPin = pin;

  // Setup buzzer output pin
  pinMode(buzzerPin, OUTPUT);

  // Set default buzzer state
  digitalWrite(buzzerPin, LOW);
}

// Buzz for given duration
void Buzzer::buzz(int buzzMs) {
  digitalWrite(buzzerPin, HIGH);
  delay(buzzMs);
  digitalWrite(buzzerPin, LOW);
}

// Buzz for given duration, then delay and buzz again
void Buzzer::doubleBuzz(int buzzMs, int delayMs) {
  buzz(buzzMs);
  delay(delayMs);
  buzz(buzzMs);
}
