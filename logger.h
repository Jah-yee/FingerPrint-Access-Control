#pragma once

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

#define MAX_LOGS 20
#define LOG_MAGIC 0xA5

struct LogRecord {
  uint8_t magic;      // 0xA5 if valid
  char name[5];       // 4 characters + null
  char time[6];       // "HH:MM"
  char date[9];       // "DD/MM/YY"
};

void loggerInit();
void logAccess(const char *name, bool granted);
int getLogCount();
LogRecord getLog(int index);

#endif
