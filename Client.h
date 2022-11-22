#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>


class Client {
private:
    SoftwareSerial * bluetooth;

public:
  Client(SoftwareSerial * _bluetooth);


  int             available(void);

  String          receive();

  void            send(const DynamicJsonDocument& payload, bool debug = true);
  void            sendError(byte idError);  
  void            begin(long speed);
};

#endif