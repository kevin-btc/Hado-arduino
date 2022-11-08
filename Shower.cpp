#include "Shower.h"

///////////////////////////////////////////////////////////////////
//
// Timer-driven callback routine called every minute.
//
///////////////////////////////////////////////////////////////////

void Shower::init(byte showerTime, byte showerShutoffTime) {
  c_showerTime = showerTime;                //in min
  c_showerShutoffTime = showerShutoffTime;  //in min
  c_isSet = true;

  Chrono chronoOpening(Chrono::MICROS, false);
  Chrono chronoPausing(Chrono::MICROS, false);
}

void Shower::set(byte showerTime, byte showerShutoffTime) {
  c_showerTime = showerTime;                //in min
  c_showerShutoffTime = showerShutoffTime;  //in min

  Shower::reset();
}

void Shower::start() {
  chronoOpening.start();

  Serial.println("start");
}

void Shower::reset() {
  chronoOpening.restart();
  chronoOpening.stop();

  chronoPausing.restart();
  chronoPausing.stop();

  chronoClosing.restart();
  chronoClosing.stop();

  Serial.println("reset");
}

void Shower::resume() {
  chronoOpening.resume();
  chronoPausing.stop();

  Serial.println("resume");
}

void Shower::stop() {
  chronoOpening.stop();
  chronoPausing.resume();

  Serial.println("stop");
}

unsigned long Shower::openingTime() {
  return chronoOpening.elapsed();  // return in ms
}

unsigned long Shower::pausingTime() {
  return chronoPausing.elapsed();  // return in ms
}

unsigned long Shower::closingTime() {
  return chronoClosing.elapsed();  // return in ms
}

bool Shower::isRunning() {
  return chronoOpening.isRunning();
}

bool Shower::isPausing() {
  return chronoPausing.isRunning();
}

bool Shower::isClosing() {
  return chronoClosing.isRunning();
}

bool Shower::isEndShowerOpening() {
  if (chronoOpening.hasPassed(c_showerTime * 60000)) {
    return true;
  }
  return false;
}

bool Shower::isEndShowerPausing() {
  if (chronoPausing.hasPassed(c_showerShutoffTime * 60000)) {
    return true;
  }
  return false;
}

bool Shower::isEndShowerClosing() {
  if (chronoClosing.hasPassed(c_showerShutoffTime * 60000)) {
    return true;
  }
  return false;
}

void Shower::update(byte *sensorPulses, bool *waterOff) {
  if (!c_isSet) {
    Serial.println("MUST_BE_SET_BEFORE");
  }

  long l_rpm = *sensorPulses;
  *sensorPulses = 0;

  if (l_rpm != 0 && !Shower::isPausing() && !Shower::isRunning()) {
    Shower::start();
  } else if (l_rpm == 0 && Shower::isRunning()) {
    Shower::stop();
  } else if (l_rpm != 0 && Shower::isPausing() && Shower::openingTime()) {
    Shower::resume();
  } else if (Shower::isRunning()) {
    if (Shower::isEndShowerOpening() && !*waterOff) {
      *waterOff = true;
      Shower::reset();
      chronoClosing.start();
      Serial.println("Closed valve");
    }
  } else if (Shower::isPausing()) {
    if (Shower::isEndShowerPausing() && *waterOff == false) {
      Shower::reset();
    } else if (Shower::openingTime() <= 60000 && Shower::pausingTime() >= 120000 && l_rpm == 0) {  // Reset if short use of water (e.g: washing hand);
      Shower::reset();
    }
  } else if (Shower::isClosing() && Shower::isEndShowerClosing()) {
    Shower::reset();
  }
}