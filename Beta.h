#ifndef _BETA_H
#define _BETA_H

#define BETA_SHOWER_MAX 6    //200
#define BETA_SHOWER_BATCH 2  //20

#include "Data.h"

class Beta {
private:
  const char* passwords[10] = {
    "9nt3yQb6",
    "NXrY2mrJ",
    "Ej1RrOlj",
    "2Sa1ZwGU",
    "FL30P2KB",
    "eydV3hOW",
    "9tWyIcYl",
    "eGcuIN3D",
    "MQJGWIWj",
    "gLD2UvEI"
  };
protected:
  bool l_isStandby;
  byte l_numberBetaShower;
  Data l_data;


public:
  void setStandby(bool standby);
  bool getStandby(void);

  byte setNumberBetaShower(void);
  byte getNumberBetaShower(void);

  void init(void);
  byte update(void);

  bool unlock(const char* password = "");
};


#endif