#pragma once

#ifndef MYTIME_H
#define MYTIME_H

bool timeConfig();
char *getTime();
char *getDate();
void setManualDateTime(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t minutes);
void setAlarm(uint8_t hr, uint8_t sec);
void disableAlarm();

#endif