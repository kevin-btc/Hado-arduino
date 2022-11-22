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

  beta.init();  // TO DELETE AFTER BETA TESTING
}

void Shower::set(byte showerTime, byte showerShutoffTime) {
  c_showerTime = showerTime;                //in min
  c_showerShutoffTime = showerShutoffTime;  //in min

  reset();
}

void Shower::start() {
  chronoOpening.start();
}

void Shower::reset() {
  chronoOpening.restart();
  chronoOpening.stop();

  chronoPausing.restart();
  chronoPausing.stop();

  chronoClosing.restart();
  chronoClosing.stop();
}

void Shower::resume() {
  chronoOpening.resume();
  chronoPausing.stop();
}

void Shower::stop() {
  chronoOpening.stop();
  chronoPausing.resume();
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

void Shower::openValve() {
  reset();
  valve.open();
}

void Shower::closeValve() {
  reset();
  chronoClosing.start();
  valve.close();
}

void Shower::monitor(byte *sensorPulses, Client *client) {
  long l_rpm = *sensorPulses;
  *sensorPulses = 0;


  if (l_rpm != 0 && !isRunning() && !isPausing() && !isClosing()) {
    start();
  } else if (l_rpm == 0 && isRunning()) {
    stop();
  } else if (l_rpm != 0 && isPausing() && openingTime()) {
    resume();
  }

  if (isRunning()) {
    if (isEndShowerOpening() && !valve.isClosed) {
      closeValve();
      // TO DELETE AFTER BETA TESTING
      byte res = beta.setNumberBetaShower();

      if (res) {
        DynamicJsonDocument doc(8);

        doc["notification"] = res;  

        client->send(doc);
      }
      /////////////////////////////////
    }
  } else if (isPausing()) {
    if (isEndShowerPausing() && !valve.isClosed) {
      reset();
    } else if (openingTime() <= 60000 && pausingTime() >= 120000 && l_rpm == 0) {  // Reset if short use of water (e.g: washing hand);
      reset();
    }
  } else if (isClosing()) {
    if (isEndShowerClosing() && valve.isClosed) {
      openValve();
    }
  }
}