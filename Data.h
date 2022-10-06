#ifndef _DATA_H
#define _DATA_H

#include <Arduino.h>

#ifndef HADO_EEPROM_START
#define HADO_EEPROM_START            0
#endif

#define HADO_EEPROM_SETUP            (HADO_EEPROM_START)
#define HADO_EEPROM_SHOWER_TIME      (HADO_EEPROM_START + 1)
#define HADO_EEPROM_SHUTOFF_TIME     (HADO_EEPROM_START + 2)
#define HADO_EEPROM_BT_MODE          (HADO_EEPROM_START + 3)
#define HADO_EEPROM_PIN_CODE         (HADO_EEPROM_START + 4)

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

    void    setBTMode(bool BTAdminMode);
    bool    getBTMode(void);

    bool    setPinCode(const String pinCode);
    String  getPinCode(void);

    void    clear();
};

#endif
