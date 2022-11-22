#ifndef _SHOWER_H
#define _SHOWER_H

#include <Arduino.h>
#include <Chrono.h>

#include "Valve.h"
#include "Beta.h"  // TO DELETE AFTER BETA-TESTING
#include "Client.h"  // TO DELETE AFTER BETA-TESTING


class Shower: Beta {
private:
  Chrono chronoOpening;
  Chrono chronoPausing;
  Chrono chronoClosing;

public:
  Valve valve;
  Beta  beta; // TO DELETE AFTER BETA-TESTING


  byte c_showerTime;
  byte c_showerShutoffTime;

  bool c_isSet = false;

  void init(byte showerTime, byte showerShutoffTime);
  void set(byte showerTime, byte showerShutoffTime);

  void start();
  void reset();
  void resume();
  void stop();

  unsigned long openingTime();
  unsigned long pausingTime();
  unsigned long closingTime();

  bool isRunning();
  bool isPausing();
  bool isClosing();

  bool isEndShowerOpening();
  bool isEndShowerPausing();
  bool isEndShowerClosing();

  void monitor(byte *sensorPulses, Client *client);

  void openValve();
  void closeValve();
};

#endif