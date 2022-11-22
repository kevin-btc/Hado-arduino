#include "hado.h"

///////////////////////////////////////////////////////////////////
//
// Setup  and loop
//
/////////////////////////////////////////////////////////////////////

void setup() {

  // Initialize Bluetooth
  client.begin(9600);
  Serial.begin(9600);

  // Initialize Hado System at the first using
  if (!data.getIsSetup()) {
    data.setShowerTime(DEFAULT_SHOWER_TIME);
    data.setShowerShutoffTime(DEFAULT_CLOSING_TIME);
    data.setPinCode(DEFAULT_PIN);
    data.setNumberShower(0);
    data.setStandby(false);
    data.setIsSetup(true);
  }
    // data.setStandby(true);

  // Initialize Shower Timer
  shower.init(data.getShowerTime(), data.getShowerShutoffTime());

  // Initialize Flowmeter
  pinMode(HAL_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HAL_SENSOR_PIN), onHallSensorEffect, RISING);

  // Start Hado System
  MonitoringTimer.start();
  handleOpenCmd();  // add etat in eeprom
}

void loop() {

  if (false && millis() - g_wakeUpTime > ACTIVITY_TIME) {
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    g_wakeUpTime = millis();
  }

  String query;
  MonitoringTimer.update();

  while (client.available() > 0) {
    query = client.receive();
  };

  if (query.length() != 0) {
    commands(query);
    delay(500);
  }
}

///////////////////////////////////////////////////////////////////
//
// Interrupt routine called when FlowMeter is generating Hal Effect pulses.
//
///////////////////////////////////////////////////////////////////

void onHallSensorEffect() {
  // Increase the number of pulses detected by the sensor
  // of the water flowmeter
  if (!g_isStandBy && !shower.beta.getStandby()) {
    g_HallSensorPulses++;
    // Serial.println("beta pulse");
  } else {
    Serial.println("pulse");
  }
}

///////////////////////////////////////////////////////////////////
//
// Bluetooth command section
//
///////////////////////////////////////////////////////////////////

bool commands(String request) {
  auto req = parseDeviceRequest(request);

  char* query = req.query.c_str();
  char* value = strchr(query, '=');

  if (value != 0) {
    *value = 0;
    value++;
  }

  if (strcmp(query, "close") == 0 && req.admin) {
    handleValveCmd(value);
  } else if (strcmp(query, "reset") == 0 && req.admin) {
    handleResetCmd(value);
  } else if (strcmp(query, "shower_time") == 0 && req.admin) {
    setShowerTime(value);
  } else if (strcmp(query, "standby") == 0 && req.admin) {
    handleStandByCmd();
  } else if (strcmp(query, "shutoff_time") == 0 && req.admin) {
    setShowerShutoffTime(value);
  } else if (strcmp(query, "pin") == 0 && req.admin) {
    setPincode(value);
  } else if (strcmp(query, "data") == 0) {
    sendDataToClient();
  } else if (strcmp(query, "unlock") == 0) {  // TO DELETE AFTER BETA TESTING
    unlockCmd(value);
  } else if (req.pinCode.length() != 0 && req.admin == false) {
    client.sendError(2);  // WRONG_PIN_CODE
  } else if (strlen(query) > 1) {
    client.sendError(1);  //COMMAND_NOT_FOUND
  }
}

DeviceRequest parseDeviceRequest(String query) {
  DeviceRequest req;

  char* pinCode = query.c_str();
  char* rawSearch = strchr(pinCode, '/');

  if (rawSearch != 0) {
    *rawSearch = 0;
    rawSearch++;

    req.query = rawSearch;
    req.pinCode = pinCode;
    req.admin = checkPinCode(pinCode);
  } else {
    req.query = query;
    req.pinCode = "";
  }

  return req;
}

bool checkPinCode(String pinCode) {
  String eepromPinCode = data.getPinCode();

  if (strcmp(pinCode.c_str(), eepromPinCode.c_str()) == 0) {
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////
//
// Send all data to Client
//
///////////////////////////////////////////////////////////////////

void sendDataToClient(bool saved) {
  DynamicJsonDocument doc(DOC_SIZE);

  JsonObject current = doc.createNestedObject("current");

  current["openingTime"] = shower.openingTime();
  current["pausingTime"] = shower.pausingTime();
  current["closingTime"] = shower.closingTime();

  doc["openingTime"] = data.getShowerTime();
  doc["closingTime"] = data.getShowerShutoffTime();
  doc["nShower"] = data.getNumberShower();

  doc["isStandBy"] = boolean(g_isStandBy);
  doc["waterOff"] = shower.valve.isClosed;

  if (saved) doc["saved"] = 1;

  serializeJson(doc, Serial);
  client.send(doc);
}

///////////////////////////////////////////////////////////////////
//
// unlock beta test // TO DELETE AFTER BETA TESTING
//
///////////////////////////////////////////////////////////////////

void unlockCmd(const char* value) {
  bool res = shower.beta.unlock(value);
  
  if (res) {
    sendDataToClient(true);
  } else {
    client.sendError(5);
  }
}



///////////////////////////////////////////////////////////////////
//
// Request to standy the monitoring
//
///////////////////////////////////////////////////////////////////

void handleStandByCmd() {
  g_isStandBy = !g_isStandBy;

  sendDataToClient(true);
}

///////////////////////////////////////////////////////////////////
//
// Request to open the valve
//
///////////////////////////////////////////////////////////////////

void handleOpenCmd() {
  g_HallSensorPulses = 0;

  shower.openValve();
  sendDataToClient(true);
}

///////////////////////////////////////////////////////////////////
//
// Request to close the valve
//
///////////////////////////////////////////////////////////////////

void handleCloseCmd() {
  g_HallSensorPulses = 0;

  shower.closeValve();
  sendDataToClient(true);
}

///////////////////////////////////////////////////////////////////
//
// Request to close/open manualy the valve
//
///////////////////////////////////////////////////////////////////

void handleValveCmd(char* value) {
  if (atoi(value) == 1) {
    handleCloseCmd();
  } else {
    handleOpenCmd();
  }
}

///////////////////////////////////////////////////////////////////
//
// Request to soft or hard reset the data
//
///////////////////////////////////////////////////////////////////

void handleResetCmd(char* value) {
  if (strcmp(value, "all") == 0 && !BETA_MODE) {
    data.clear();
  } else {
    shower.openValve();
    shower.reset();
  }

  sendDataToClient();
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

  data.setShowerTime(showerTime);

  int showerShutoffTime = data.getShowerShutoffTime();

  shower.set(showerTime, showerShutoffTime);
  sendDataToClient(true);
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

  data.setShowerShutoffTime(showerShutoffTime);

  int showerTime = data.getShowerTime();

  shower.set(showerTime, showerShutoffTime);
  sendDataToClient(true);
}

///////////////////////////////////////////////////////////////////
//
// shower Shutoff Time Command
//
///////////////////////////////////////////////////////////////////

void setPincode(char* value) {
  data.setPinCode(value);
  sendDataToClient(true);
}