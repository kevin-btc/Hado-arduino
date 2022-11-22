#include "Beta.h"

void Beta::init() {
  l_isStandby = l_data.getStandby();
  l_numberBetaShower = l_data.getNumberShower();

  Serial.println("init l_isStandby : " + String(l_isStandby));
  Serial.println("init l_numberBetaShower : " + String(l_numberBetaShower));
}

void Beta::setStandby(bool standby) {
  Serial.println("before l_isStandby : " + String(l_isStandby));

  l_isStandby = standby;

  Serial.println("after l_isStandby : " + String(l_isStandby));


  l_data.setStandby(standby);
}

bool Beta::getStandby(void) {
  return l_isStandby;
}

byte Beta::setNumberBetaShower(void) {
  Serial.println("before l_numberBetaShower : " + String(l_numberBetaShower));

  l_numberBetaShower += 1;

  Serial.println("after l_numberBetaShower : " + String(l_numberBetaShower));


  l_data.setNumberShower(l_numberBetaShower);
  return update();
}

byte Beta::getNumberBetaShower(void) {
  return l_numberBetaShower;
}

bool Beta::unlock(const char* password = "") {
  byte batch = l_numberBetaShower / BETA_SHOWER_BATCH;

  Serial.println("unlock l_isStandby : " + String(l_isStandby));
  Serial.println("unlock comp pass : " + String(strcmp(passwords[batch], password) == 0));
  Serial.println("unlock pass : " + String(passwords[batch]));
  Serial.println("unlock query : " + String(password));
  Serial.println("unlock batch : " + String(batch));

  if (batch && l_isStandby && strcmp(passwords[batch - 1], password) == 0) {
    setStandby(false);
    return true;
  }

  return false;
}

byte Beta::update() {
  if (l_numberBetaShower % BETA_SHOWER_BATCH == 0 && l_numberBetaShower >= BETA_SHOWER_BATCH && l_numberBetaShower <= BETA_SHOWER_MAX) {
    setStandby(true);
    Serial.println("lock");
    
    return 1;
  }
  if (l_numberBetaShower > BETA_SHOWER_MAX) {
    Serial.println("unlock impossible");

    return 2;
  }

  return 0;
}