// tell them to be cleaning the sensor
#include <EEPROM.h>
#include "biometrics.h"
#include "buttons.h"
#include <STM32RTC.h>

/* Get the rtc object */
STM32RTC &rtc = STM32RTC::getInstance();


#define VALID_MAGIC 0xA5  // to know if a memory sector is availabe for overwriting

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

#define MAX_USERS 300

struct UserRecord {
  uint8_t magic;                          // 1 byte
  uint8_t id;                             // 1 bytes Fingerprint ID from sensor (e.g., 1 to 50) also serve as eeprom address
  char name[5] = { '@', '@', '@', '@' };  // 5 bytes User name (e.g., "Alex")
  uint8_t notMaster = 1;                  //1 byte: 0 means user is a master
};


//const char modes[4] = {"access","enroll","delete","log"};
char *mode = "access";   // start with access control mode
bool doorLocked = true;  // door is locked initially.
bool UsersExists = false;
int enrolledUsers = 0;


int writeName();  // handle name input
bool userExist(uint16_t userID, UserRecord &user);
int getEnrolledCount();
int getEEPROMAddress(uint16_t userID);
void saveUser(UserRecord &user);
void timeConfig();
char *getTime();
char *getDate();
char *getUserName();
UserRecord getUser(uint16_t id);
char name_buff[5] = { '@', '@', '@', '@' };
int8_t cursorPos = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  timeConfig();
  buttonConfig();
  while (!checkDisplay())
    ;
  //displayLogo();
  delay(5000);
  //EEPROM.update(0, 0xFF);
  Serial.println("starting");
  debugf("EEPROM size: %i\n", EEPROM.length());

  pinMode(DOOR_PIN, OUTPUT);

  // for (int i = 27; i <= 30; i++) {
  //   pinMode(i, INPUT_PULLUP);  // set all button pins as input
  // }

  digitalWrite(DOOR_PIN, 1);  //lock the door
                              // show loading animation with my logo
                              // play welcome sound
  delay(100);
  biometricsSetup();
  // show lock icon

  // check if any user exist
  // if yes set mode to enroll
  enrolledUsers = getEnrolledCount();
  if (enrolledUsers == 0) {
    mode = "enroll";
    Serial.println("No users found. Entering enroll mode.");
  } else {
    UsersExists = true;
    debugf("found %i user(s)", enrolledUsers);
  }

  debugln(getTime());
  debugln(getDate());
}
/*
if no user, check if master 

figure out access, then come back to enroll
*/
void loop() {
  // put your main code here, to run repeatedly:
  if (mode == "menu") {
    debugln("in menu");
    displayMenu();
    delay(500);
    while (1) {
      if (RIGHT_BUTTON.wasPressed()) {
        mode = "log";
        debugln("Selected: log");
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
    }
  }

  while (mode == "enroll") {

    UserRecord user;
    // get number of available records
    uint8_t current_id = 1 + enrolledUsers;
    // if > 299 print no space icon
    if (current_id > MAX_USERS) {
      debugln("no space for new enrollment");
      displayTwoWords("no", "space");
      mode = "access";
      debugln("access mode");
      delay(1000);
      break;
    }

    // do enroll stuff
    debugln("Ready to enroll a fingerprint!");
   user.id = current_id;
    // if no records then make the person master during enroll
    if (!UsersExists) {
      user.notMaster = 0;  // make master
      debugln("user is a master");
   
      // ask for name and input name
      getUserName(user.name);
      user.name[sizeof(user.name) - 1] = '\0';  // Ensure null-termination
    }
    // else ask if the person should be made master

    else {

      // check if it is a master that is trying to enroll
      while (1) {
        display_three_words("Place", "Master", "Finger", 2);
        SensorRsp rsp = getFingerprintID();
        if (!rsp.isError) {
          char *endptr;
          // Convert base 10 string to long, then cast to int
          uint16_t id = (uint16_t)strtol(rsp.msg, &endptr, 10);
          UserRecord user = getUser(id);
          if (user.notMaster == 0) {
            displayTwoWords("Master", user.name);
            delay(1000);
            break;
          } else {
            displayTwoWords("Not", "Master");
            delay(1000);
          }
        } else {
          debugf("sensor err: %s\n", rsp.msg);
        }
      }

          // ask for name and input name
    getUserName(user.name);
    user.name[sizeof(user.name) - 1] = '\0';  // Ensure null-termination
    
      displayOption("Master??");
      while (1) {
        char *option = choice();
        if (option == "yes") {
          user.notMaster = 0;
          debugf("%s is master", user.name);
          displayTwoWords("Master", user.name);
          delay(1000);
          break;
        } else if (option == "no") {
          user.notMaster = 1;
          displayTwoWords("Not", "Master");
          delay(1000);
          break;
        }
      }
    }
    while (1) {
      uint8_t success = getFingerprintEnroll(current_id);
      debugf("enrollment result %i\n", success);
      if (success == 1) {
        saveUser(user);
        enrolledUsers++;
        Serial.println("saved");
        break;
      } else if (success == 0xAA) {
        debugln("cancled");
        mode = "access";
        break;
      }
      debugln("failed to enroll");
      delay(2000);  // replace with millis() later
    }

    while (1) {
      if (mode != "enroll") {
        break;
      }
      Serial.println("ENROLL AGAIN? Y/N");
      displayOption("Again??");
      char *option = choice();
      if (option == "yes") {
        break;
      } else if (option == "no") {
        mode = "access";
        break;
      }
    }
  }

  while (mode == "access") {
    //Serial.println("hello access");
    displayTwoWords("Place", "Finger");  // replace with finger image or animation
    if (RIGHT_BUTTON.wasPressed()) {
      mode = "menu";
      break;
    }

    SensorRsp rsp = getFingerprintID();
    if (!rsp.isError) {
      debugf("user with id %s unlocked the door\n", rsp.msg);
    } else {
      debugf("sensor err: %s\n", rsp.msg);
    }
  }

  while (mode == "delete") {
    Serial.println("hello delete");
  }

  while (mode == "log") {
    Serial.println("hello log");
  }

  while (mode == "locked") {
    Serial.println("hello locked");
  }
}


int getEnrolledCount() {
  int count = 0;
  UserRecord user;

  for (int id = 1; id <= MAX_USERS; id++) {
    if (userExist(id, user)) {  // Returns user.active
      count++;
    }
  }
  Serial.printf("count: %i\n", count);
  return count;
}



bool userExist(uint16_t userID, UserRecord &user) {
  int addr = getEEPROMAddress(userID);
  EEPROM.get(addr, user);
  // Serial.printf("userID: %i, user.id: %i\n",userID,user.id);
  // Serial.printf("user.active: %i\n",user.active);
  return (user.magic == VALID_MAGIC);  // Returns false if slot is unassigned
}

UserRecord getUser(uint16_t id) {
  UserRecord user;
  int addr = getEEPROMAddress(id);
  EEPROM.get(addr, user);
  return user;
}

int getEEPROMAddress(uint16_t userID) {
  return (userID - 1) * sizeof(UserRecord);
  // each user takes 24bytes, so the next
  // user will be 24 bytes away from the
  // previous user
}


void saveUser(UserRecord &user) {
  user.magic = VALID_MAGIC;
  int addr = getEEPROMAddress(user.id);
  EEPROM.put(addr, user);  // Writes directly to internal flash emulation

  Serial.print("Saved user ");
  Serial.print(user.name);
  Serial.print(" at address ");
  Serial.println(addr);
}

void timeConfig() {
  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
  // By default the LSI is selected as source.
  rtc.setClockSource(STM32RTC::LSE_CLOCK);  // LSE is more accurate for clock based functions
  rtc.begin();                              // initialize RTC 24H format
  if (!rtc.isTimeSet()) {
    Serial.println("time is not set, setting time");
    byte hours, minutes, seconds;
    sscanf(__TIME__, "%d:%d:%d", &hours, &minutes, &seconds);

    //  Parse __DATE__ string (Format: "Mmm DD YYYY", e.g., "Aug 14 2026")
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

    // 3. Update the RTC
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(5, day, month, shortYear);  // Day of week defaults to 1 here
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

void getUserName(char (&name)[5]) {
  while (1) {
    displayTextInput(name, cursorPos);
    if (RIGHT_BUTTON.wasPressed()) {
      cursorPos++;
      if (cursorPos > 3) {
        cursorPos = 0;
      }
      Serial.printf("cursor at pos: %i\n", cursorPos);
    }

    if (UP_BUTTON.wasPressed()) {
      char character = name[cursorPos] + 1;
      if (character > 90) {
        character = '@';
      }
      name[cursorPos] = character;

    } else if (DOWN_BUTTON.wasPressed()) {
      char character = name[cursorPos] - 1;
      if (character < 65) {
        character = '@';
      }
      name[cursorPos] = character;
    }
    if (LEFT_BUTTON.wasPressed()) {
      //save
      return;
    }
  }
}
