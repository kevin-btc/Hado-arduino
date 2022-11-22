#ifndef _VALVE_H_
#define _VALVE_H_

#define VALVE_ENABLE_PIN 6
#define VALVE_OPEN_PIN 7
#define VALVE_CLOSE_PIN 8


#include <Arduino.h>

class Valve {
public:
  Valve();

  bool isClosed;

  void open(void);
  void close(void);
};

#endif