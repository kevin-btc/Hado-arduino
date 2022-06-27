#include "Data.h"


void Data::printAll() {
  Serial.println("  - Shower Time        : " + String(getShowerTime()));
  Serial.println("  - Shower ShutoffTime : " + String(getShowerShutoffTime()));
  Serial.println("  - Setup              : " + String(getIsSetup()));
};

void Data::setIsSetup(byte isSetup) {
  EEPROM.write(HADO_EEPROM_SETUP, isSetup);
}

bool Data::getIsSetup() {
  return EEPROM.read(HADO_EEPROM_SETUP);
}

void Data::setShowerTime(byte showerTime) {
  EEPROM.write(HADO_EEPROM_SHOWER_TIME, showerTime);
}

byte Data::getShowerTime() {
  return EEPROM.read(HADO_EEPROM_SHOWER_TIME);
}

void Data::setShowerShutoffTime(byte showerShutoffTime) {
  EEPROM.write(HADO_EEPROM_SHUTOFF_TIME, showerShutoffTime);
}

byte Data::getShowerShutoffTime() {
  return EEPROM.read(HADO_EEPROM_SHUTOFF_TIME);
}

bool Data::setPinCode(const String pinCode) {

  bool isCorrectPinCode = true;

  for (int i = 0; i < LEN_PIN_CODE ; i++)
  {
    if (!isalnum(pinCode[i])) {
      isCorrectPinCode = false;
    }
  }

  if (isCorrectPinCode) {
    for (int i = 0; i < LEN_PIN_CODE; i++)
    {
      EEPROM.write(HADO_EEPROM_PIN_CODE + i, pinCode[i]);
    }
  }

  return isCorrectPinCode;
}

String Data::getPinCode() {
  char pinCode[LEN_PIN_CODE + 1];

  for (int i = 0; i < LEN_PIN_CODE; i++)
  {
    pinCode[i] = EEPROM.read(HADO_EEPROM_PIN_CODE + i);
  }
  
  pinCode[LEN_PIN_CODE] = '\0';

  return String(pinCode);
}

void Data::setBTMode(bool BTAdminMode) {
  EEPROM.write(HADO_EEPROM_BT_MODE, BTAdminMode);
}

bool Data::getBTMode() {
  return EEPROM.read(HADO_EEPROM_BT_MODE);
}

void Data::clear() {
  void(* resetFunc) (void) = 0;

  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }

  resetFunc();
}
