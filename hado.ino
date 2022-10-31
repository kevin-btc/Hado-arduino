#include "hado.h"

///////////////////////////////////////////////////////////////////
//
// Setup  and loop
//
/////////////////////////////////////////////////////////////////////

void setup() {

  // Initialize Serial port / Bluetooth

  Serial.begin(9600);

  client.begin(9600);

  // Initialize ElectroValve

  valve.init();

  // Initialize Hado System

  g_isSetup = data.getIsSetup();

  if (g_isSetup) {
    g_showerShutoffTime = data.getShowerShutoffTime();
    g_showerTime = data.getShowerTime();
    g_monitor_mins = g_showerTime + DELTA_MONITOR;

  } else {
    data.setShowerTime(g_showerTime);
    data.setShowerShutoffTime(g_showerShutoffTime);
    data.setPinCode("H4D0");
    data.setIsSetup(true);
  }

  handleOpenCmd();  // add etat in eeprom

  monitor.init(g_monitor_mins, g_showerTime, g_showerShutoffTime);
  monitor.setRpms(g_monitor_mins);

  attachInterrupt(digitalPinToInterrupt(HAL_SENSOR_PIN), onHallSensorEffect, RISING);

  MonitoringTimer.start();
}

void loop() {
  //  digitalWrite(13, HIGH);

  if (false && millis() - g_wakeUpTime > g_activityTime) {
    //    digitalWrite(13, LOW);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    //    digitalWrite(13, HIGH);

    g_wakeUpTime = millis();
  }

  String query = "";

  MonitoringTimer.update();

  while (client.available()) {
    query = client.receive();
  }

  while (Serial.available()) {
    query = Serial.readString();
  }

  if (query.length() != 0) {
    commands(query);
  }

  valveClosingTimer(query);
}

///////////////////////////////////////////////////////////////////
//
// Interrupt routine called when FlowMeter is generating Hal Effect pulses.
//
///////////////////////////////////////////////////////////////////

void onHallSensorEffect() {
  // Increase the number of pulses detected by the sensor
  // of the water flowmeter
  if (!g_isStandBy) {
    g_HallSensorPulses++;
    Serial.println("sensor");
  }
}

///////////////////////////////////////////////////////////////////
//
// Close water valve by controlling 1 servomotor to ball valve
//
///////////////////////////////////////////////////////////////////

void closeWaterValve() {
  g_waterOff = true;

  valve.close();

  g_HallSensorPulses = 0;
}

///////////////////////////////////////////////////////////////////
//
// Open water valve by controlling 1 servomotor to ball valve
//
///////////////////////////////////////////////////////////////////

void openWaterValve() {
  g_waterOff = false;

  valve.open();

  g_HallSensorPulses = 0;
}

void valveClosingTimer(String query) {


  if (g_waterOff) {
    handleCloseCmd();
    for (int t = 0; t < g_showerShutoffTime * MINUTE_IN_MS; t += 200) {
      query = "";
      while (client.available()) {
        query = client.receive();
      }

      if (query.length() != 0) {
        commands(query);
        if (!g_waterOff) {
          return;
        }
      }
      delay(200);
    }
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

  while (search != NULL) {

    char* value = strchr(search, '=');
    if (value != 0) {
      *value = 0;
      value++;
    }

    bool isCorrectPinCode = checkPinCode(ret.pinCode);

    if (strcmp(CLOSE, search) == 0 && isCorrectPinCode) {
      handleValveCmd(value);
    } else if (strcmp(RESET, search) == 0 && isCorrectPinCode) {
      handleResetCmd(value);
    } else if (strcmp(SHOWER_TIME, search) == 0 && isCorrectPinCode) {
      setShowerTime(value);
    } else if (strcmp(STAND_BY, search) == 0 && isCorrectPinCode) {
      handleStandByCmd();
    } else if (strcmp(SHUTOFF_TIME, search) == 0 && isCorrectPinCode) {
      setShowerShutoffTime(value);
    } else if (strcmp(TIME, search) == 0) {
      getCurrentShowerTime(true);
    } else if (strcmp(DATA, search) == 0) {
      getData();
    } else if (!isCorrectPinCode){
      client.sendError(2);
    } else {
      client.sendError(1);  //COMMAND_NOT_FOUND
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

  if (strcmp(pinCode.c_str(), eepromPinCode.c_str()) == 0) {
    return true;
  }

  return false;
}

void getData() {
  DynamicJsonDocument doc(64);

  JsonObject current = doc.createNestedObject("current");
  current["openingTime"] = getCurrentShowerTime(false);
  current["closingTime"] = millis() - (g_currentClosingTime / 1000 / 60);

  doc["closingTime"] = data.getShowerShutoffTime();
  doc["openingTime"] = data.getShowerTime();
  doc["isSetup"] = g_isSetup;
  doc["waterOff"] = g_waterOff;
  doc["isStandBy"] = g_isStandBy;

  client.send(doc);
}

///////////////////////////////////////////////////////////////////
//
// standBy requested
//
///////////////////////////////////////////////////////////////////

void handleStandByCmd() {
  DynamicJsonDocument doc(16);

  g_isStandBy = !g_isStandBy;

  doc["isStandBy"] = g_isStandBy;

  client.send(doc);
}

///////////////////////////////////////////////////////////////////
//
// Open requested
//
///////////////////////////////////////////////////////////////////

void handleOpenCmd() {
  DynamicJsonDocument doc(8);

  openWaterValve();

  doc["waterOff"] = 0;
  g_currentClosingTime = 0;

  client.send(doc);
  delay(2000);
}

///////////////////////////////////////////////////////////////////
//
// Close requested
//
///////////////////////////////////////////////////////////////////

void handleCloseCmd() {
  DynamicJsonDocument doc(16);
  g_currentClosingTime = millis();

  closeWaterValve();

  doc["waterOff"] = 1;
  doc["closedAt"] = g_currentClosingTime;

  client.send(doc);
  delay(2000);
}

///////////////////////////////////////////////////////////////////
//
// Manualy close/open requested
//
///////////////////////////////////////////////////////////////////

void handleValveCmd(char* value) {
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

void handleResetCmd(char* value) {
  DynamicJsonDocument doc(8);

  doc["reset"] = 0;

  if (strcmp(value, "all") == 0) {
    doc["reset"] = 1;
    data.clear();
  } else {
    doc["reset"] = 2;
    monitor.clearRpms();
  }

  client.send(doc);
}

///////////////////////////////////////////////////////////////////
//
// shower Time Command
//
///////////////////////////////////////////////////////////////////

void setShowerTime(char* value) {
  int showerTime = atoi(value);

  if (showerTime < 1 || showerTime >= 255) {
    client.sendError(3);  // MUST_BE_BETWEEN_1_AND_255
    return;
  }
  MonitoringTimer.stop();

  data.setShowerTime(showerTime);

  g_showerTime = data.getShowerTime();
  g_monitor_mins = g_showerTime + DELTA_MONITOR;

  monitor.update(g_monitor_mins, g_showerTime, g_showerShutoffTime);

  DynamicJsonDocument doc(8);

  doc["saved"] = 1;

  client.send(doc);

  MonitoringTimer.start();
}

///////////////////////////////////////////////////////////////////
//
// shower Shutoff Time Command
//
///////////////////////////////////////////////////////////////////

void setShowerShutoffTime(char* value) {
  int showerShutoffTime = atoi(value);

  if (showerShutoffTime < 1 || showerShutoffTime >= 255) {
    client.sendError(3);  // MUST_BE_BETWEEN_1_AND_255
    return;
  }

  MonitoringTimer.stop();

  data.setShowerShutoffTime(showerShutoffTime);
  g_showerShutoffTime = showerShutoffTime;

  monitor.update(g_monitor_mins, g_showerTime, g_showerShutoffTime);

  DynamicJsonDocument doc(8);

  doc["saved"] = 1;

  client.send(doc);

  MonitoringTimer.start();
}


///////////////////////////////////////////////////////////////////
//
// Current Shower time
//
///////////////////////////////////////////////////////////////////

byte getCurrentShowerTime(bool toSend) {
  byte showerTime = 0;
  byte* rpms = monitor.getRpms();

  for (int i = 0; i < g_monitor_mins; i++) {
    if (rpms[i]) {
      showerTime++;
    }
  }

  if (toSend) {
    DynamicJsonDocument doc(8);

    doc["actualShowerTime"] = showerTime;

    client.send(doc);
  }

  MonitoringTimer.start();

  return showerTime;
}