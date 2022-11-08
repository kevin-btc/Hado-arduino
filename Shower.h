#ifndef _SHOWER_H
#define _SHOWER_H

#include <Arduino.h>
#include <Chrono.h>

class Shower {
private:
  Chrono chronoOpening;
  Chrono chronoPausing;
  Chrono chronoClosing;

public:
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

  void update(byte *sensorPulses, bool *waterOff);
};

#endif