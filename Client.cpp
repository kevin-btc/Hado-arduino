#include "HardwareSerial.h"
#include "Client.h"

Client::Client(SoftwareSerial* _bluetooth) {
  bluetooth = _bluetooth;
}

void Client::begin(long speed) {
  bluetooth->begin(speed);
}

int Client::available() {
  return bluetooth->available();
}

String Client::receive() {
  return bluetooth->readString();
}
// MAX ERROR NUMBER: 255 (BYTE)
// Error 0 : "BLUETOOTH_UNVAILABLE"
// Error 1 : "COMMAND_NOT_FOUND"
// Error 2 : "WRONG_PIN_CODE"
// Error 3 : "MUST_BE_BETWEEN_1_AND_255"


void Client::sendError(byte idError) {
  DynamicJsonDocument doc(8);

  doc["error"] = idError;

  serializeJson(doc, Serial);
  serializeJson(doc, *bluetooth);
}

void Client::send(const DynamicJsonDocument& payload, bool debug = true) {
  // if (debug && Serial.available()) {
    serializeJson(payload, Serial);
  // }

  if (bluetooth->isListening()) {
    serializeJson(payload, *bluetooth);
  } else {
    sendError(0);
  }
}