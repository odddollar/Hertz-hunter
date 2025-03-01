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
void Buzzer::buzz(int durationMs) {
  digitalWrite(buzzerPin, HIGH);
  delay(durationMs);
  digitalWrite(buzzerPin, LOW);
}
