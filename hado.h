#ifndef _HADO_H_
#define _HADO_H_

#include <SoftwareSerial.h>
#include <LowPower.h>
#include <Ticker.h>

#include "Data.h"
#include "Client.h"
#include "Shower.h"

#define HAL_SENSOR_PIN 2

#define BT_PIN_1 10
#define BT_PIN_2 11

#define DOC_SIZE 64

#define TIMER_PERIOD 1000.0  // 60 secondes

#define DEFAULT_SHOWER_TIME 2
#define DEFAULT_CLOSING_TIME 2

#define ACTIVITY_TIME 30
#define DEFAULT_PIN "H4D0"

#define BETA_MODE true


///////////////////////////////////////////////////////////////////
//
// Global variables
//
///////////////////////////////////////////////////////////////////

bool g_isStandBy = false;

// global variables accessed from ISR that need to be protected
volatile byte g_HallSensorPulses = 0;  // FlowMeter pulses that have occurred in the current minute

unsigned long g_wakeUpTime = millis();

///////////////////////////////////////////////////////////////////
//
// Global Class
//
///////////////////////////////////////////////////////////////////

SoftwareSerial Bluetooth(BT_PIN_1, BT_PIN_2);
Client client(&Bluetooth);

Data data;
Shower shower;

Ticker MonitoringTimer([] {
  shower.monitor(&g_HallSensorPulses, &client);
},
                       TIMER_PERIOD);

struct DeviceRequest {
  String pinCode;
  String query;
  bool admin;
} typedef DeviceRequest;

void sendDataToClient(bool saved = false);

#endif