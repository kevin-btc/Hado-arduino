#ifndef _HADO_H_
#define _HADO_H_

#include  <VarSpeedServo.h>
#include  <ArduinoJson.h>
#include  <LowPower.h>
#include  <EEPROM.h>
#include  <Ticker.h>

#include  "SoftwareSerial.h"
#include  "Data.h"
#include  "Monitor.h"


#define HAL_SENSOR_PIN        2    // Flowmeter Hal sensor is connected to this pin
#define TIMER_PERIOD          60000.0 // 60 secondes. Period required to calculate RPM of Hal effect sensor in ms
#define DELTA_MONITOR         5
#define MINUTE_IN_MS          60000

#define OPEN_POSITION         0
#define CLOSE_POSITION        95
#define SPEED_VALVE           50

#define VALVE_CTRL_PIN        12
#define WAIT_END_MOVE         true

#define BTN_PIN               4

#define BT_PIN_1              10
#define BT_PIN_2              11

#define CLOSE                 "close"
#define RESET                 "reset"
#define TIME                  "time"
#define MENU                  "menu"
#define SHOWER_TIME           "shower_time"
#define SHUTOFF_TIME          "shutoff_time"
#define PIN_CODE              "pin"

///////////////////////////////////////////////////////////////////
//
// Global variables
//
///////////////////////////////////////////////////////////////////

byte                     g_showerShutoffTime = 2; // Duration valve will be closed to warn time is up
byte                     g_showerTime = 7; // Duration shower time
byte                     g_monitor_mins = g_showerTime + DELTA_MONITOR; // Duration control
byte*                    g_Rpms;     // History of rpms in the last "n" minutes

bool                     checkIfSetup = false;
bool                     g_isSetup = false;
bool                     g_isStandBy = false;


// global variables accessed from ISR that need to be protected
volatile bool            g_waterOff = false;     // True if valve needs to be closed temporarily
volatile byte            g_HallSensorPulses = 0;   // FlowMeter pulses that have occurred in the current minute


///////////////////////////////////////////////////////////////////
//
// Global Class
//
///////////////////////////////////////////////////////////////////


Monitor                 monitor;

Ticker                   MonitoringTimer([] {monitor.onTimerTick( &g_HallSensorPulses, &g_waterOff);}, TIMER_PERIOD );      // Interupt timer used for counting minutes
SoftwareSerial           SerialBT(BT_PIN_1, BT_PIN_2);
VarSpeedServo            Valve;
Data                     data;

struct QueryAndPinCode {
    String pinCode;
    String query;
} typedef QueryAndPinCode;

void  closeWaterValve();
void  openWaterValve();
void  onHallEffect();
void  onTimerTick();

#endif
