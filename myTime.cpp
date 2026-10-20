#include <Arduino.h>
#include <STM32RTC.h>
#include "myTime.h"
#include "door.h"

#define DEBUG 1

#include "debugMacro.h"

/* Get the rtc object */
STM32RTC &rtc = STM32RTC::getInstance();

bool timeConfig() {
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  debugln("LSE CLOCK IS BEEN USED");
  rtc.begin();
  if (!rtc.isTimeSet()) {
    debugln("time is not set");
    return false;
  }
 debugln("time has been set");
 return true;
}

char *getTime() {
  static char time[6];
  snprintf(time, sizeof(time), "%02d:%02d", rtc.getHours(), rtc.getMinutes());
  return time;
}

char *getDate() {
  static char date[9];
  snprintf(date, sizeof(date), "%02d/%02d/%02d", rtc.getDay(), rtc.getMonth(), rtc.getYear());
  return date;
}

void setManualDateTime(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t minutes) {
  rtc.setTime(hours, minutes, 0);
  rtc.setDate(day, month, year);
  Serial.printf("Manual RTC set: %02d/%02d/%02d %02d:%02d:00\n", day, month, year, hours, minutes);
}

void alarmMatch(void *data)
{
  UNUSED(data);
  debugln("Alarm fired!");
lockDoor();
rtc.disableAlarm();
}

void setAlarm(uint8_t hr, uint8_t sec){
    rtc.attachInterrupt(alarmMatch);
  rtc.setAlarmDay(rtc.getDay());
  rtc.setAlarmTime(hr, sec, 0, 123);
  rtc.enableAlarm(rtc.MATCH_DHHMMSS); 
}

void disableAlarm(){
rtc.disableAlarm();
}

