#include "buzzer.h"

Buzzer::Buzzer(uint8_t p)
  : pin(p) {

  // Setup buzzer output pin
  pinMode(pin, OUTPUT);

  // Set default buzzer state
  digitalWrite(pin, LOW);
}

// Single buzz with programmed period
void Buzzer::buzz() {
  xTaskCreate(_buzz, "buzz", BUZZER_STACK_SIZE, this, 1, NULL);
}

// Double buzz with programmed period
void Buzzer::doubleBuzz() {
  xTaskCreate(_doubleBuzz, "buzz", BUZZER_STACK_SIZE, this, 1, NULL);
}

// Spawned in another thread to prevent blocking
void Buzzer::_buzz(void *parameter) {
  // Static cast weirdness to access pin variable
  Buzzer *buzzer = static_cast<Buzzer *>(parameter);

  digitalWrite(buzzer->pin, HIGH);
  vTaskDelay(pdMS_TO_TICKS(BUZZ_DURATION));
  digitalWrite(buzzer->pin, LOW);

  // Delete current task
  vTaskDelete(NULL);
}

// Spawned in another thread to prevent blocking
void Buzzer::_doubleBuzz(void *parameter) {
  // Static cast weirdness to access pin variable
  Buzzer *buzzer = static_cast<Buzzer *>(parameter);

  digitalWrite(buzzer->pin, HIGH);
  vTaskDelay(pdMS_TO_TICKS(BUZZ_DURATION));
  digitalWrite(buzzer->pin, LOW);

  vTaskDelay(pdMS_TO_TICKS(BUZZ_DELAY));

  digitalWrite(buzzer->pin, HIGH);
  vTaskDelay(pdMS_TO_TICKS(BUZZ_DURATION));
  digitalWrite(buzzer->pin, LOW);

  // Delete current task
  vTaskDelete(NULL);
}
