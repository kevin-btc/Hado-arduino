#include "Valve.h"


Valve::Valve() {
  pinMode(VALVE_CLOSE_PIN, OUTPUT);
  pinMode(VALVE_OPEN_PIN, OUTPUT);
  pinMode(VALVE_ENABLE_PIN, OUTPUT);
  digitalWrite(VALVE_ENABLE_PIN, HIGH);
}

void Valve::open() {
  isClosed = false;
  
  digitalWrite(VALVE_CLOSE_PIN, LOW);
  digitalWrite(VALVE_OPEN_PIN, LOW);

  digitalWrite(VALVE_OPEN_PIN, HIGH);

  delay(500);
  digitalWrite(VALVE_OPEN_PIN, LOW);
}

void Valve::close() {
  isClosed = true;

  digitalWrite(VALVE_CLOSE_PIN, LOW);
  digitalWrite(VALVE_OPEN_PIN, LOW);

  digitalWrite(VALVE_CLOSE_PIN, HIGH);

  delay(500);

  digitalWrite(VALVE_CLOSE_PIN, LOW);
}