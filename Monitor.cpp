#include "Monitor.h"

///////////////////////////////////////////////////////////////////
//
// Timer-driven callback routine called every minute.
//
///////////////////////////////////////////////////////////////////


void Monitor::setRpms(byte monitor_mins) {

  if (c_rpms) {
    free(c_rpms);
  }

  c_rpms = (byte*) malloc (sizeof(byte) * (monitor_mins + 1));

  memset( c_rpms, 0, monitor_mins );
}

byte* Monitor::getRpms() {
  return c_rpms;
}

void Monitor::clearRpms() {
  memset( c_rpms, 0, c_monitor_mins );
}


void Monitor::init(byte monitor_mins, byte showerTime) {
  c_monitor_mins = monitor_mins;
  c_showerTime = showerTime;
  c_isSet = true;

  Monitor::setRpms(monitor_mins);

}


void Monitor::onTimerTick(byte *sensorPulses, bool *waterOff) {
  if (!c_isSet) {
    Serial.println("MUST_BE_SET_BEFORE");
  }

  Serial.println("Timer Tick");

  cli();                          // disable interupts
  long l_rpm = *sensorPulses;     // copy rpm value locally
  *sensorPulses = 0;              // sensor reading done for this minute
  sei();                          // reenable interrupts

  memmove( &c_rpms[1], &c_rpms[0], c_monitor_mins - sizeof( c_rpms[0] ) );

  c_rpms[0] = l_rpm;

  int l_numUsedTicks = 0;

  for ( int idx = 0; idx < c_monitor_mins; idx++ )
  {
    if ( c_rpms[idx] )
    {
      l_numUsedTicks++;
    }
  }

  if ( l_numUsedTicks >= c_showerTime && !*waterOff )
  {
    *waterOff = true;
    clearRpms();
  } else if (l_numUsedTicks >= 1 && l_rpm == 0 ) {
    clearRpms();
  }
}
