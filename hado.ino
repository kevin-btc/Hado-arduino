#include "hado.h"

///////////////////////////////////////////////////////////////////
//
// Setup  and loop
//
/////////////////////////////////////////////////////////////////////

void setup()
{

  // Initialize Serial port / Bluetooth

  SerialBT.begin(9600);
  Serial.begin(9600);

  // Initialize Hado System

  g_isSetup = data.getIsSetup();

  if (g_isSetup)
  {
    g_showerShutoffTime = data.getShowerShutoffTime();
    g_showerTime = data.getShowerTime();
    g_monitor_mins = g_showerTime + DELTA_MONITOR;

  } else {
    data.setShowerTime(g_showerTime);
    data.setShowerShutoffTime(g_showerShutoffTime);
    data.setPinCode("H4D0");
    data.setIsSetup(true);
  }

  handleOpenCmd(); // add etat in eeprom

  monitor.init(g_monitor_mins, g_showerTime);
  monitor.setRpms(g_monitor_mins);

  attachInterrupt( digitalPinToInterrupt(HAL_SENSOR_PIN), onHallSensorEffect, RISING);

  MonitoringTimer.start();
}

void loop()
{

  String query = "";

  MonitoringTimer.update();

  while (SerialBT.available())
  {
    query = SerialBT.readString();
    Serial.println(query);
  }

  while (Serial.available())
  {
    query = Serial.readString();
    SerialBT.write(Serial.read());
  }

  if (query.length() != 0)
  {
    commands(query);
  }

  valveClosingTimer(query);
}

///////////////////////////////////////////////////////////////////
//
// Interrupt routine called when FlowMeter is generating Hal Effect pulses.
//
///////////////////////////////////////////////////////////////////

void onHallSensorEffect()
{
  // Increase the number of pulses detected by the sensor
  // of the water flowmeter
  g_HallSensorPulses++;
  Serial.println("sensor");
}

///////////////////////////////////////////////////////////////////
//
// Close water valve by controlling 1 servomotor to ball valve
//
///////////////////////////////////////////////////////////////////

void closeWaterValve()
{
  g_waterOff = true;

  Valve.attach(VALVE_CTRL_PIN);
  Valve.write(CLOSE_POSITION, SPEED_VALVE, WAIT_END_MOVE);

  while (Valve.isMoving());

  Valve.detach();

  g_HallSensorPulses = 0;
}

///////////////////////////////////////////////////////////////////
//
// Open water valve by controlling 1 servomotor to ball valve
//
///////////////////////////////////////////////////////////////////

void openWaterValve()
{
  g_waterOff = false;

  Valve.attach(VALVE_CTRL_PIN);
  Valve.write(OPEN_POSITION, SPEED_VALVE, WAIT_END_MOVE);

  while (Valve.isMoving());

  Valve.detach();

  g_HallSensorPulses = 0;
}

void valveClosingTimer(String query) {
  if ( g_waterOff )
  {
    handleCloseCmd();

    int leftTimeInSeconde = 0;

    byte seconds = 0;
    byte minutes = 0;

    for (int t = 0 ; t < g_showerShutoffTime * MINUTE_IN_MS ; t += 1000)
    {
      query = "";
      while (SerialBT.available())
      {
        query = SerialBT.readString();
        Serial.println(query);
      }

      if (query.length() != 0)
      {
        commands(query);
        if (!g_waterOff) {
          return;
        }
      } else {

        leftTimeInSeconde = (g_showerShutoffTime * MINUTE_IN_MS - t) / 1000 ;

        seconds = leftTimeInSeconde % 60;
        minutes = leftTimeInSeconde / 60;
        SerialBT.println("OPENING IN " + String(minutes) + ":" + String(seconds) + "Seconde");

      }

      delay(1000);
    }
    SerialBT.println("Exit boucle");
    handleOpenCmd();
  }
}



///////////////////////////////////////////////////////////////////
//
// Bluetooth command section
//
///////////////////////////////////////////////////////////////////


bool commands(String query) {
  auto ret = splitPinCodeFromQuery(query);

  char* search = strtok(&ret.query[0], "&");

  while ( search != NULL ) {

    char *value = strchr(search, '=');
    if (value != 0) {
      *value = 0;
      value++;
    }

    if ( strcmp(CLOSE, search) == 0 && checkPinCode(ret.pinCode)) {
      handleValveCmd(value);
    } else if (strcmp(RESET, search) == 0) {
      handleResetCmd(value);
    } else if (strcmp(SHOWER_TIME , search) == 0 && checkPinCode(ret.pinCode)) {
      showerTimeCmd(value);
    } else if (strcmp(SHUTOFF_TIME, search) == 0 && checkPinCode(ret.pinCode)) {
      showerShutoffTimeCmd(value);
    } else if (strcmp(TIME, search) == 0) {
      getCurrentShowerTime();
    } else if (strcmp(MENU, search) == 0) {
      handleRoot();
    } else {
      debug();
    }

    search = strtok(NULL, "&");
  }
}

QueryAndPinCode splitPinCodeFromQuery(String query) {

  QueryAndPinCode ret;

  char* pinCode = query.c_str();
  char* rawSearch = strchr(pinCode, '/');

  if (rawSearch != 0) {
    *rawSearch = 0;
    rawSearch++;

    ret.query = rawSearch;
    ret.pinCode = pinCode;
  } else {
    ret.query = query;
    ret.pinCode = "";
  }

  return ret;
}

bool checkPinCode(String pinCode) {
  String eepromPinCode = data.getPinCode();

  bool isCorrectPinCode =  false;

  if (strcmp(pinCode.c_str(), eepromPinCode.c_str()) == 0) {
    isCorrectPinCode = true;
  } else {
    SerialBT.println(F("WRONG_PIN_CODE"));
  }

  return isCorrectPinCode;
}

void handleRoot()
{
  String message;

  message += g_waterOff ? "Valve is Closed\n\n" : "Valve is Opened\n\n";
  message += "Shower time : " + String(g_showerTime) + "\n";
  message += "Shower shutoff time : " + String(g_showerShutoffTime) + "\n";
  message += "Monitor control time : " + String(g_monitor_mins) + "\n";
  message += "Setup ended : " + String(g_isSetup ? "true\n" : "false\n");
  message += "\nHELP :\n";
  message += "-> 1 for close valve\n";
  message += "-> 2 for open valve\n";
  message += "-> 3 for reset HADO system\n";
  message += "-> 4 for get current shower time\n";
  message += "-> 5 for show this menu\n";
  message += "-> 6 for new shower time\n";
  message += "-> 7 for new shower shutoff time\n";
  message += "-> 8 for show all parameters\n";
  message += "g_HallSensorPulses : " + String(g_HallSensorPulses) + "\n";
  message += "g_waterOff : " + String(g_waterOff) + "\n";
  message += "g_monitor_mins : " + String(g_monitor_mins) + "\n";



  SerialBT.println(message);
}


void debug()
{
  String message;

  message += g_waterOff ? "Valve is Closed\n\n" : "Valve is Opened\n\n";
  message += "Shower time : " + String(g_showerTime) + "\n";
  message += "Shower shutoff time : " + String(g_showerShutoffTime) + "\n";
  message += "Monitor control time : " + String(g_monitor_mins) + "\n";
  message += "Setup ended : " + String(g_isSetup ? "true\n" : "false\n");
  message += "g_HallSensorPulses : " + String(g_HallSensorPulses) + "\n";
  message += "g_waterOff : " + String(g_waterOff) + "\n";
  message += "g_monitor_mins : " + String(g_monitor_mins) + "\n";



  SerialBT.println(message);
}

///////////////////////////////////////////////////////////////////
//
// Open requested
//
///////////////////////////////////////////////////////////////////

void handleOpenCmd()
{
  openWaterValve();
  Serial.println(F("WATER_OPENED"));
  SerialBT.println(F("WATER_OPENED"));

  delay(2000);
}

///////////////////////////////////////////////////////////////////
//
// Close requested
//
///////////////////////////////////////////////////////////////////

void handleCloseCmd()
{
  closeWaterValve();
  Serial.println(F("WATER_CLOSED"));
  SerialBT.println(F("WATER_CLOSED"));

  delay(2000);
}

///////////////////////////////////////////////////////////////////
//
// Manualy close/open requested
//
///////////////////////////////////////////////////////////////////

void handleValveCmd(char * value)
{
  if (atoi(value) == 1) {
    g_waterOff = true;
  } else {
    handleOpenCmd();
  }
}

///////////////////////////////////////////////////////////////////
//
// Reset requested
//
///////////////////////////////////////////////////////////////////

void handleResetCmd(char * value)
{

  if (strcmp(value, "all") == 0) {
    data.clear();
  } else {
    monitor.clearRpms();
  }

  Serial.println(F("RESET_MONITORING"));
  SerialBT.println(F("RESET_MONITORING"));

  handleOpenCmd();
}

///////////////////////////////////////////////////////////////////
//
// shower Time Command
//
///////////////////////////////////////////////////////////////////

void showerTimeCmd(char* value)
{
  int showerTime = atoi(value);

  if ( showerTime < 1 || showerTime >= 255)
  {
    SerialBT.println(F("MUST_BE_BETWEEN_1_AND_255"));
    return;
  }
  MonitoringTimer.stop();

  data.setShowerTime(showerTime);

  g_showerTime = data.getShowerTime();
  g_monitor_mins = g_showerTime + DELTA_MONITOR;

  monitor.setRpms(g_monitor_mins);

  SerialBT.println("SHOWER_TIME_SAVED");

  MonitoringTimer.start();
}

///////////////////////////////////////////////////////////////////
//
// shower Shutoff Time Command
//
///////////////////////////////////////////////////////////////////

void showerShutoffTimeCmd(char* value)
{
  int showerShutoffTime = atoi(value);

  if ( showerShutoffTime < 1 || showerShutoffTime >= 255)
  {
    SerialBT.println(F("MUST_BE_BETWEEN_1_AND_255"));
    return;
  }

  MonitoringTimer.stop();

  data.setShowerShutoffTime(showerShutoffTime);
  g_showerShutoffTime = showerShutoffTime;

  SerialBT.println("SUCCESS_SHOWER_SHUTOFF_TIME");

  MonitoringTimer.start();
}


///////////////////////////////////////////////////////////////////
//
// Current Shower time
//
///////////////////////////////////////////////////////////////////

void getCurrentShowerTime()
{
  byte showerTime = 0;
  byte* rpms = monitor.getRpms();

  for (int i = 0; i < g_monitor_mins; i++)
  {
    if (rpms[i])
    {
      showerTime++;
    }
  }

  SerialBT.println("SHOWER_TIME : " + String(showerTime));
}
