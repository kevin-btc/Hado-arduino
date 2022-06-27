#ifndef   _MONITOR_H
#define   _MONITOR_H

#include  <Arduino.h>


class Monitor {
  public:
    byte              c_monitor_mins;
    byte              c_showerTime;

    byte*             c_rpms;
    byte*             getRpms(void);

    void              onTimerTick(byte *sensorPulses, bool *waterOff);
    void              init(byte monitor_mins, byte showerTime);
    void              setRpms(byte monitor_mins);
    void              clearRpms(void);

    bool              c_isSet = false;
};

#endif
