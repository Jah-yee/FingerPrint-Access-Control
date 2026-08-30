// tell them to be cleaning the sensor
#include <EEPROM.h>
#include "biometrics.h"
#include "buttons.h"
#include <STM32RTC.h>

/* Get the rtc object */
STM32RTC &rtc = STM32RTC::getInstance();


#define VALID_MAGIC 0xA5

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
  uint8_t magic;          // 1 byte
  uint8_t id;             // 1 bytes Fingerprint ID from sensor (e.g., 1 to 50) also serve as eeprom address
  char name[5];           // 5 bytes User name (e.g., "Alex")
  uint8_t notMaster = 1;  //1 byte: 0 means user is a master
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

  for (int i = 27; i <= 30; i++) {
    pinMode(i, INPUT_PULLUP);  // set all button pins as input
  }

  digitalWrite(DOOR_PIN, 0);  //lock the door
                              // show loading animation with my logo
                              // play welcome sound

  // adafruit finger print set up functions
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
  clearDisplay();
}
/*
if no user, check if master 

figure out access, then come back to enroll
*/
void loop() {
  // put your main code here, to run repeatedly:
  while (mode == "menu") {
    debugln("in menu");
    displayMenu();
    while(1){
      
    }
    // uint8_t button = getButtonsState();

    // if (button == UP) {
    //   mode = "enroll";
    //   break;
    // }

    // else if (button == DOWN) {
    //   mode = "access";
    //   break;
    // }

    // else if (button == LEFT) {
    //   mode = "delete";
    //   break;
    // }

    // else if (button == RIGHT) {
    //   mode = "log";
    //   break;
    // }
  }

  while (mode == "enroll") {
    UserRecord user;
    // get number of available records
    uint8_t current_id = 1 + enrolledUsers;
    // if > 299 print no space icon
    if (current_id > MAX_USERS) {
      Serial.println("no space for new enrollment");
      mode = "access";
      Serial.println("access mode");
      break;
    }

    // do enroll stuff
    Serial.println("Ready to enroll a fingerprint!");

    user.id = current_id;
    // ask for name and input name
    strncpy(user.name, "defa", sizeof(user.name) - 1);
    user.name[sizeof(user.name) - 1] = '\0';  // Ensure null-termination
    // user.name = writeName()
    // if no records then make the person master during enroll
    if (!UsersExists) {
      user.notMaster = 0;  // make master
      Serial.println("user is a master");
    }
    // else ask if the person should be made master
    else {
      Serial.println("Master?");
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
      // uint8_t button = getButtonsState();
      // if (button == RIGHT) {
      //   debugln("N");
      //   break;
      // }
      // if (button == LEFT) {
      //   debugln("Y");
      //   mode = "access";
      //   debugln("entering access mode");
      //   break;
      // }
    }
    // save details to sensor and eeprom
    //exit enrolling using left button
    // mode="access";
    // break;
  }

  while (mode == "access") {
    //Serial.println("hello access");
    displayTwoWords("Place", "Finger");  // replace with finger image or animation
    if (RIGHT_BUTTON.state == true) {
      LEFT_BUTTON.state = false;
      mode = "menu";
      break;
    }
    // uint8_t button = getButtonsState();

    // if (button == RIGHT) {
    //   mode = "menu";
    //   break;
    // }
    SensorRsp rsp = getFingerprintID();
    if (!rsp.isError) {
      debugf("user with id %s unlocked the door\n", rsp.msg);
    } else {
      debugf("sensor err: %s\n", rsp.msg);
    }

    //debugf("%i button pressed\n", button);
    //delay(1000);
    // show lock icon
    // do check
    // if wrong show invalid fingerprint icon
    // if correct show unlock icon and name
    // unlock the door
    // log it
    // wait 5 secs
    // lock the door
    // if exist is pressed
  }

  while (mode == "delete") {
    Serial.println("hello delete");
    // uint8_t button = getButtonsState();

    // if (button == RIGHT) {
    //   mode = "menu";
    //   break;
    // }
    //int id=1;
    // get total records

    // when right button is pressed increment Id and show corresponding record
    // when left button
    // id cannot be less than 1
    // id cannot be greater than number of records
    // exit button
  }

  while (mode == "log") {
    Serial.println("hello log");
    // uint8_t button = getButtonsState();

    // if (button == RIGHT) {
    //   mode = "menu";
    //   break;
    // }
    // check if master if not locked the device
    // just show log icon
    // if cmd is get log send log, delete from mem and exit
    // if exit is pressed break
  }

  while (mode == "locked") {
    Serial.println("hello locked");
    // check if master
    // if master, unlock door and switch to access mode
    // else show message: not master
  }
  // int up = digitalRead(PB12);    //red
  // int down = digitalRead(PB13);  //blue
  // Serial.printf("up button state: %i\ndown button state: %i\n", up, down);
  // delay(500);
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

/*
get button pressed
writeNAme
deleting and buttons next
screens 
 enroll: should have an exit button
 access screen
 delete screen
 

icons
 lock icon
 unlock icon
 invalid fingerprint
 logo
menu states
- access control
- enroll - only masters, as if person should be a master
- remove access - only masters
- export log - export via usb serial port
*/
