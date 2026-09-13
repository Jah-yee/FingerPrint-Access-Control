// tell them to be cleaning the sensor
#include "biometrics.h"
#include "buttons.h"
#include "users.h"
#include "myTime.h"
#include "logger.h"

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(...) Serial.printf(__VA_ARGS__)
#else
#define debug(x)
#define debugln(x)
#define debugf(...)
#endif

#define SPEAKER_PIN PA15
#define DOOR_PIN PC13

const char *EnrollModeHeader = "Enroll Mode";
const char *AccessModeHeader = "Access Mode";
const char *DeleteModeHeader = "Delete Mode";
const char *LogModeHeader    = "Log Mode";

char *mode = "access";   // start with access control mode
uint8_t menuPage = 1;    // main menu page (1 or 2)
bool doorLocked = true;  // door is locked initially.
bool UsersExists = false;

bool exit();
void lockDoor();
void unlockDoor();
bool verifyMasterPermission(const char *modeHeader);

void setup() {
  Serial.begin(115200);
  timeConfig();
  buttonConfig();
  loggerInit();

  while (!checkDisplay())
    ;
  delay(1000);
// deleteAllPrints();
// deleteAll();
  Serial.println("starting");

  pinMode(DOOR_PIN, OUTPUT);
  lockDoor();

  biometricsSetup();

  enrolledUsers = getEnrolledCount();
  if (enrolledUsers == 0) {
    mode = "enroll";
    Serial.println("No users found. Entering enroll mode.");
  } else {
    UsersExists = true;
    debugf("found %i user(s)\n", enrolledUsers);
  }

  debugln(getTime());
  debugln(getDate());
}

void loop() {
  //checked
  if (mode == "menu") {
    debugf("in menu (page %d)\n", menuPage);
    displayMenu(menuPage);
    delay(300);
    while (mode == "menu") {
      if (menuPage == 1) {
        if (RIGHT_BUTTON.wasPressed()) {
          menuPage = 2;
          debugln("Switched to Menu Page 2");
          break;
        } else if (UP_BUTTON.wasPressed()) {
          mode = "enroll";
          debugln("Selected: enroll");
          break;
        } else if (DOWN_BUTTON.wasPressed()) {
          mode = "access";
          debugln("Selected: access");
          break;
        } else if (LEFT_BUTTON.wasPressed()) {
          mode = "delete";
          debugln("Selected: delete");
          break;
        }
      } else { // menuPage == 2
        if (RIGHT_BUTTON.wasPressed()) {
          menuPage = 1;
          debugln("Switched back to Menu Page 1");
          break;
        } else if (UP_BUTTON.wasPressed()) {
          mode = "log";
          debugln("Selected: log");
          break;
        } else if (DOWN_BUTTON.wasPressed()) {
          mode = "settime";
          debugln("Selected: settime");
          break;
        }
      }
    }
  }
//checked
  /////////////////////////////////////// Enroll Mode //////////////////////////////////
  if (mode == "enroll") {
    if (UsersExists && !verifyMasterPermission(EnrollModeHeader)) {
      mode = "menu";
    }

    while (mode == "enroll") {
      if (exit()) { break; }
      UserRecord user;
      uint16_t current_id = getNextAvailableID();
      if (current_id == 0 || current_id > MAX_USERS) {
        debugln("no space for new enrollment");
        displayTwoWords("no", "space", EnrollModeHeader);
        mode = "menu";
        delay(1500);
        break;
      }

      debugln("Ready to enroll a fingerprint!");
      user.id = current_id;

      if (!UsersExists) {
        user.notMaster = 0; // First user is Master
        debugln("First user initialized as Master");
        getUserName(user.name);
        user.name[sizeof(user.name) - 1] = '\0';
      } else {
        getUserName(user.name);
        user.name[sizeof(user.name) - 1] = '\0';

        // Ask if new user should be made a master
        while (mode == "enroll") {
          displayOption("Master??");
          char *option = choice();
          if (strcmp(option, "yes") == 0) {
            user.notMaster = 0;
            debugf("%s is set as master\n", user.name);
            displayTwoWords("Master", user.name, EnrollModeHeader);
            delay(1000);
            break;
          } else if (strcmp(option, "no") == 0) {
            user.notMaster = 1;
            displayTwoWords("Not", "Master", EnrollModeHeader);
            delay(1000);
            break;
          }
        }
      }

      // Enroll new user's fingerprint on AS608
      while (mode == "enroll") {
        if (exit()) { break; }
        uint8_t success = getFingerprintEnroll(current_id);
        debugf("enrollment result %i\n", success);
        if (success == FINGERPRINT_OK) {
          saveUser(user);
          enrolledUsers++;
          UsersExists = true;
          debugln("saved user to EEPROM");
          break;
        }
        debugln("failed to enroll");
        delay(1500);
      }

      while (mode == "enroll") {
        if (exit()) { break; }
        displayOption("Again??");
        char *option = choice();
        if (strcmp(option, "yes") == 0) {
          break;
        } else if (strcmp(option, "no") == 0) {
          mode = "access";
          break;
        }
      }
    }
  }
//checked
  /////////////////////////////////////// Access Mode //////////////////////////////////
  if (mode == "access") {
    while (mode == "access") {
      if (exit()) { break; }
      displayTwoWords("Place", "Finger", AccessModeHeader);
      SensorRsp rsp = getFingerprintID();
      if (!rsp.isError) {
        debugf("user with id %s unlocked the door\n", rsp.msg);
        char *endptr;
        uint16_t id = (uint16_t)strtol(rsp.msg, &endptr, 10);
        UserRecord user = getUser(id);
        displayTwoWords("Welcome", user.name, AccessModeHeader);
        logAccess(user.name, true);// log only successful entry
        unlockDoor();
        delay(4000);
        lockDoor();
      } else {
        debugf("sensor err: %s\n", rsp.msg);
        if (strcmp(rsp.msg, "no match") == 0) {
          displayWord("Invalid", AccessModeHeader);
          delay(1000);
        }
      }
    }
  }
  //checked
  /////////////////////////////////////// Delete Mode //////////////////////////////////
  if (mode == "delete") {
    if (!verifyMasterPermission(DeleteModeHeader)) {
      mode = "menu";
    }

    while (mode == "delete") {
      if (exit()) { break; }
      displayCustomMenu("Query Opt", "U-Scroll", "D-Name");
      
      if (UP_BUTTON.wasPressed()) {
        debugln("Scroll mode selected");
        if (enrolledUsers == 0) {
          displayTwoWords("No", "Users", DeleteModeHeader);
          delay(1500);
          continue;
        }

        uint16_t activeIDs[MAX_USERS];
        int activeCount = 0;
        for (uint16_t i = 1; i <= MAX_USERS; i++) {
          UserRecord u;
          if (userExist(i, u)) {
            activeIDs[activeCount++] = i;
          }
        }

        int currentIndex = 0;
        while (mode == "delete") {
          if (exit()) { break; }
          if (activeCount == 0) {
            displayTwoWords("No", "Users", DeleteModeHeader);
            delay(1500);
            break;
          }

          UserRecord user = getUser(activeIDs[currentIndex]);
          char buffer[12];
          snprintf(buffer, sizeof(buffer), "Del %s", user.name);
          displayOption(buffer, "exit>", "<DEL");

          if (UP_BUTTON.wasPressed()) {
            currentIndex = (currentIndex + 1) % activeCount;
          } else if (DOWN_BUTTON.wasPressed()) {
            currentIndex = (currentIndex - 1 + activeCount) % activeCount;
          } else if (LEFT_BUTTON.wasPressed()) {
            if (user.notMaster == 0 && getMasterCount() <= 1) {
              debugln("Blocked deletion of sole Master user.");
              displayTwoWords("Cant't", "Delete", DeleteModeHeader);
              delay(1500);
              continue;
            }
            deleteFingerprint(user.id);
            deleteUser(user);
            displayTwoWords(user.name, "deltd", DeleteModeHeader);
            delay(1500);

            // refresh active IDs
            activeCount = 0;
            for (uint16_t i = 1; i <= MAX_USERS; i++) {
              UserRecord checkU;
              if (userExist(i, checkU)) {
                activeIDs[activeCount++] = i;
              }
            }
            if (currentIndex >= activeCount) {
              currentIndex = activeCount > 0 ? activeCount - 1 : 0;
            }
          }
        }
      } else if (DOWN_BUTTON.wasPressed()) {
        debugln("Name search delete selected");
        char targetName[5] = {'@', '@', '@', '@', '\0'};
        getUserName(targetName);
        targetName[4] = '\0';
        
        int deletedCount = 0;
        for (uint16_t i = 1; i <= MAX_USERS; i++) {
          UserRecord u;
          if (userExist(i, u)) {
            if (strncmp(u.name, targetName, 4) == 0) {
              if (u.notMaster == 0 && getMasterCount() <= 1) {
                debugln("Blocked deletion of sole Master user during name search.");
                displayTwoWords("Can'r", "Delete", DeleteModeHeader);
                delay(1500);
                continue;
              }
              deleteFingerprint(u.id);
              deleteUser(u);
              deletedCount++;
            }
          }
        }

        if (deletedCount > 0) {
          displayTwoWords(targetName, "deltd", DeleteModeHeader);
          delay(1500);
        } else {
          displayTwoWords("Not", "Found", DeleteModeHeader);
          delay(1500);
        }
      }
    }
  }
  //checked
  /////////////////////////////////////// Log Mode //////////////////////////////////
  if (mode == "log") {
    if (!verifyMasterPermission(LogModeHeader)) {
      mode = "menu";
    }

    while (mode == "log") {
      if (exit()) { break; }
      int totalLogs = getLogCount();
      if (totalLogs == 0) {
        displayTwoWords("No", "Logs", LogModeHeader);
        delay(1500);
        mode = "menu";
        break;
      }

      int currentLogIdx = 0;
      while (mode == "log") {
        if (exit()) { break; }
        LogRecord rec = getLog(currentLogIdx);
        displayLogEntry(rec.name, rec.date, rec.time, currentLogIdx + 1, totalLogs);

        if (UP_BUTTON.wasPressed()) {
          currentLogIdx = (currentLogIdx + 1) % totalLogs;
        } else if (DOWN_BUTTON.wasPressed()) {
          currentLogIdx = (currentLogIdx - 1 + totalLogs) % totalLogs;
        }
      }
    }
  }

  /////////////////////////////////////// Set Time Mode //////////////////////////////////
  if (mode == "settime") {
    if (!verifyMasterPermission("Set Time")) {
      mode = "menu";
    }

    uint8_t hours = 12;
    uint8_t minutes = 0;
    uint8_t day = 1;
    uint8_t month = 1;
    uint8_t year = 26;

    // Parse current time & date
    int hVal = 0, mVal = 0, dVal = 1, monVal = 1, yVal = 26;
    if (sscanf(getTime(), "%d:%d", &hVal, &mVal) == 2) {
      hours = (uint8_t)hVal;
      minutes = (uint8_t)mVal;
    }
    if (sscanf(getDate(), "%d/%d/%d", &dVal, &monVal, &yVal) == 3) {
      day = (uint8_t)dVal;
      month = (uint8_t)monVal;
      year = (uint8_t)yVal;
    }

    uint8_t fieldPos = 0; // 0=Hours, 1=Minutes, 2=Day, 3=Month, 4=Year

    while (mode == "settime") {
      displayTimeInput(day, month, year, hours, minutes, fieldPos);

      if (RIGHT_BUTTON.wasPressed()) {
        fieldPos = (fieldPos + 1) % 5;
        debugf("Time input active field: %d\n", fieldPos);
      }

      if (UP_BUTTON.wasPressed()) {
        if (fieldPos == 0)      hours = (hours + 1) % 24;
        else if (fieldPos == 1) minutes = (minutes + 1) % 60;
        else if (fieldPos == 2) day = (day % 31) + 1;
        else if (fieldPos == 3) month = (month % 12) + 1;
        else if (fieldPos == 4) year = (year + 1) % 100;
      } else if (DOWN_BUTTON.wasPressed()) {
        if (fieldPos == 0)      hours = (hours == 0) ? 23 : hours - 1;
        else if (fieldPos == 1) minutes = (minutes == 0) ? 59 : minutes - 1;
        else if (fieldPos == 2) day = (day <= 1) ? 31 : day - 1;
        else if (fieldPos == 3) month = (month <= 1) ? 12 : month - 1;
        else if (fieldPos == 4) year = (year == 0) ? 99 : year - 1;
      }

      if (LEFT_BUTTON.wasPressed()) {
        setManualDateTime(day, month, year, hours, minutes);
        displayTwoWords("Time", "Set", "Set Time");
        delay(1500);
        mode = "menu";
        break;
      }
    }
  }

}
bool verifyMasterPermission(const char *modeHeader) {
  if (enrolledUsers == 0) {
    return true; // Bypass master check if no users exist
  }

  debugf("Verifying Master permission for mode: %s\n", modeHeader);
  display_three_words("Place", "Master", "Finger", 2);

  while (1) {
    if (exit()) {
      debugln("Master check cancelled by user.");
      return false;
    }

    SensorRsp rsp = getFingerprintID();
    if (!rsp.isError) {
      char *endptr;
      uint16_t id = (uint16_t)strtol(rsp.msg, &endptr, 10);
      UserRecord master = getUser(id);
      if (master.notMaster == 0) {
        debugf("Master verified: %s\n", master.name);
        displayTwoWords("Master", master.name, modeHeader);
        delay(1000);
        return true;
      } else {
        debugln("Scanned user is not a Master.");
        displayTwoWords("Not", "Master", modeHeader);
        delay(1000);
        display_three_words("Place", "Master", "Finger", 2);
      }
    } else {
      if (strcmp(rsp.msg, "no match") == 0) {
        debugln("No match found for Master verification.");
        displayTwoWords("No", "Match", modeHeader);
        delay(1000);
        display_three_words("Place", "Master", "Finger", 2);
      }
    }
    delay(50);
  }
}

bool exit() {
  bool buttonState = RIGHT_BUTTON.wasPressed();
  if (buttonState) {
    mode = "menu";
  }
  return buttonState;
}

void lockDoor() {
  digitalWrite(DOOR_PIN, 0);  // lock the door
}

void unlockDoor() {
  digitalWrite(DOOR_PIN, 1);  // unlock the door
}