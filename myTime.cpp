#include <Arduino.h>
#include <STM32RTC.h>
#include "myTime.h"

/* Get the rtc object */
STM32RTC &rtc = STM32RTC::getInstance();

void timeConfig() {
  // Select RTC clock source if available, or default via begin()
#if defined(LSE_CLOCK)
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  debugln("LSE CLOCK IS BEEN USED");
#elif defined(LSI_CLOCK)
  rtc.setClockSource(STM32RTC::LSI_CLOCK);
  debugln("LSI CLOCK IS BEEN USED");
#endif
  rtc.begin();                              // initialize RTC 24H format

  if (!rtc.isTimeSet()) {
    Serial.println("time is not set, setting time");
    byte hours, minutes, seconds;
    sscanf(__TIME__, "%d:%d:%d", &hours, &minutes, &seconds);

    // Parse __DATE__ string (Format: "Mmm DD YYYY", e.g., "Aug 14 2026")
    char monthStr[4];
    int day, year;
    sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);

    // Convert 3-letter month to a number (1-12)
    int month = 1;
    const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    for (int i = 0; i < 12; i++) {
      if (strcmp(monthStr, months[i]) == 0) {
        month = i + 1;
        break;
      }
    }

    // Convert 4-digit year to 2-digit year for the RTC library
    int shortYear = year % 100;

    // Update the RTC (day, month, year)
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(day, month, shortYear);
  }
  Serial.println("time has been set");
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