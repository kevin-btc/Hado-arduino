#ifndef _DATA_H
#define _DATA_H

#include <Arduino.h>

#ifndef HADO_EEPROM_START
#define HADO_EEPROM_START            0
#endif

#define HADO_EEPROM_SETUP            (HADO_EEPROM_START)
#define HADO_EEPROM_SHOWER_TIME      (HADO_EEPROM_START + 1)
#define HADO_EEPROM_SHUTOFF_TIME     (HADO_EEPROM_START + 2)
#define HADO_EEPROM_PIN_CODE         (HADO_EEPROM_START + 4)

#define HADO_EEPROM_BETA_STANDBY     (HADO_EEPROM_START + 3) // to delete after beta-test
#define HADO_EEPROM_BETA_SHOWER      (HADO_EEPROM_START + 8) // to delete after beta-test

#define LEN_PIN_CODE                 4

class Data {
  public:
    void    printAll(void);
    
    void    setIsSetup(byte isSetup);
    bool    getIsSetup();

    void    setShowerTime(byte showerTime);
    byte    getShowerTime();

    void    setShowerShutoffTime(byte showerShutoffTime);
    byte    getShowerShutoffTime();

    bool    setPinCode(const String pinCode);
    String  getPinCode(void);

    void    clear();

    void    setStandby(bool standby); // to delete after beta-test
    bool    getStandby(void); // to delete after beta-test

    void    setNumberShower(byte numberBetaShower); // to delete after beta-test
    byte    getNumberShower(void); // to delete after beta-test
};

#endif
