#ifndef _HADO_H_
#define _HADO_H_

#include <SoftwareSerial.h>
#include <LowPower.h>
#include <Ticker.h>

#include "Data.h"
#include "Monitor.h"
#include "Valve.h"
#include "Client.h"

#define TIMER_PERIOD 60000.0  // 60 secondes
#define DELTA_MONITOR 5
#define MINUTE_IN_MS 60000

#define HAL_SENSOR_PIN 2

#define CLOSE "close"
#define RESET "reset"
#define TIME "time"
#define MENU "menu"
#define DATA "data"
#define SHOWER_TIME "shower_time"
#define SHUTOFF_TIME "shutoff_time"
#define PIN_CODE "pin"
#define STAND_BY "standby"

#define BT_PIN_1 10
#define BT_PIN_2 11

#define DOC_SIZE 250

///////////////////////////////////////////////////////////////////
//
// Global variables
//
///////////////////////////////////////////////////////////////////

byte g_showerShutoffTime = 2;                        // Duration valve will be closed to warn time is up
byte g_showerTime = 7;                               // Duration shower time
byte g_monitor_mins = g_showerTime + DELTA_MONITOR;  // Duration control

bool checkIfSetup = false;
bool g_isSetup = false;
bool g_isStandBy = false;

// global variables accessed from ISR that need to be protected
volatile bool g_waterOff = false;      // True if valve needs to be closed temporarily
volatile byte g_HallSensorPulses = 0;  // FlowMeter pulses that have occurred in the current minute

unsigned long g_wakeUpTime = millis();
unsigned long g_currentClosingTime = 0;
unsigned long g_activityTime = 180000;  // 30 min * 60s * 1000ms;


///////////////////////////////////////////////////////////////////
//
// Global Class
//
///////////////////////////////////////////////////////////////////

Monitor monitor;
SoftwareSerial Bluetooth(BT_PIN_1, BT_PIN_2);
Client client(&Bluetooth);

Ticker MonitoringTimer([] {
  monitor.onTimerTick(&g_HallSensorPulses, &g_waterOff);
},
                       TIMER_PERIOD);  // Interupt timer used for counting minutes
Data data;
Valve valve;

struct DeviceRequest {
  String  pinCode;
  String  query;
  bool    admin;
} typedef DeviceRequest;

void closeWaterValve();
void openWaterValve();
void onHallEffect();
void onTimerTick();

#endif