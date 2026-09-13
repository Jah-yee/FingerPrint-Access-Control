#include <EEPROM.h>
#include "logger.h"
#include "users.h"
#include "myTime.h"
#include "debugMacro.h"

// Base address in EEPROM after max user records (300 * sizeof(UserRecord))
static const int LOG_EEPROM_START_ADDR = MAX_USERS * sizeof(UserRecord); // 3000

static int logHead = 0;

static int getLogEEPROMAddress(int index) {
  return LOG_EEPROM_START_ADDR + (index * sizeof(LogRecord));
}

void loggerInit() {
  // Find head index or count existing valid logs
  int count = 0;
  for (int i = 0; i < MAX_LOGS; i++) {
    LogRecord logRec;
    EEPROM.get(getLogEEPROMAddress(i), logRec);
    if (logRec.magic == LOG_MAGIC) {
      count++;
    }
  }
  debugf("Logger initialized. Found %d existing log entries.\n", count);
}

void logAccess(const char *name, bool granted) {
  LogRecord logRec;
  logRec.magic = LOG_MAGIC;
  
  // Copy name (up to 4 chars + null)
  strncpy(logRec.name, name, 4);
  logRec.name[4] = '\0';
  
  // Get time and date
  strncpy(logRec.time, getTime(), sizeof(logRec.time) - 1);
  logRec.time[sizeof(logRec.time) - 1] = '\0';
  
  strncpy(logRec.date, getDate(), sizeof(logRec.date) - 1);
  logRec.date[sizeof(logRec.date) - 1] = '\0';
  

  // Find next insertion slot (circular buffer)
  // We can scan or use a static head index
  int addr = getLogEEPROMAddress(logHead);
  EEPROM.put(addr, logRec);
  
  debugf("Logged access: %s %s [%s %s] at slot %d\n",
         logRec.name, "GRANTED" ,
         logRec.date, logRec.time, logHead);

  logHead = (logHead + 1) % MAX_LOGS;
}

int getLogCount() {
  int count = 0;
  for (int i = 0; i < MAX_LOGS; i++) {
    LogRecord logRec;
    EEPROM.get(getLogEEPROMAddress(i), logRec);
    if (logRec.magic == LOG_MAGIC) {
      count++;
    }
  }
  return count;
}

LogRecord getLog(int index) {
  // Return Nth valid log (0 = newest available, count-1 = oldest)
  LogRecord validLogs[MAX_LOGS];
  int validCount = 0;
  
  for (int i = 0; i < MAX_LOGS; i++) {
    LogRecord logRec;
    EEPROM.get(getLogEEPROMAddress(i), logRec);
    if (logRec.magic == LOG_MAGIC) {
      validLogs[validCount++] = logRec;
    }
  }

  if (index >= 0 && index < validCount) {
    // Return in reverse order (newest first)
    return validLogs[validCount - 1 - index];
  }

  // Return empty record if out of range
  LogRecord emptyRec;
  emptyRec.magic = 0;
  strncpy(emptyRec.name, "NONE", 4);
  emptyRec.name[4] = '\0';
  emptyRec.time[0] = '\0';
  emptyRec.date[0] = '\0';
  return emptyRec;
}
