// tell them to be cleaning the sensor
#include "biometrics.h"
#include "buttons.h"
#include "users.h"
#include "myTime.h"

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
const char *DeleteModeHeader= "Delete Mode";
char *mode = "access";   // start with access control mode
bool doorLocked = true;  // door is locked initially.
bool UsersExists = false;


bool exit();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  timeConfig();
  buttonConfig();
  while (!checkDisplay())
    ;
  delay(5000);

  Serial.println("starting");

  pinMode(DOOR_PIN, OUTPUT);
  lockDoor();
  // show loading animation with my logo
  // play welcome sound
  delay(100);
  biometricsSetup();
  // show lock icon
// deleteAll();
// deleteAllPrints();
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
  /////////////////////////////////////// Enroll Mode //////////////////////////////////
  while (mode == "enroll") {
    if (exit()) { break; }  // a function to allow user to exit this menu
    UserRecord user;
    // get number of available records
    uint16_t current_id = 1 + enrolledUsers;
    // if > 299 print no space icon
    if (current_id > MAX_USERS) {
      debugln("no space for new enrollment");
      displayTwoWords("no", "space", EnrollModeHeader);
      mode = "menu";
      debugln(mode);
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

    else {
      /*
      Sequence
      * ask for master permission before enrolling new users
      * if master, enroll the new user.
      */

      while (mode == "enroll") {
        if (exit()) { break; }  // a function to allow user to exit this menu
        // need's master's permission to enroll users
        display_three_words("Place", "Master", "Finger", 2);
        SensorRsp rsp = getFingerprintID();
        if (!rsp.isError) {
          char *endptr;
          // Convert base 10 string to long, then cast to int
          uint16_t id = (uint16_t)strtol(rsp.msg, &endptr, 10);
          UserRecord master = getUser(id);
          if (master.notMaster == 0) {
            displayTwoWords("Master", user.name, EnrollModeHeader);
            delay(1000);

            // ask for new user's name and input name
            getUserName(user.name);
            user.name[sizeof(user.name) - 1] = '\0';  // Ensure null-termination

            break;
          } else {
            displayTwoWords("Not", "Master", EnrollModeHeader);
            delay(1000);
          }
        } else {
          debugf("sensor err: %s\n", rsp.msg);
          if (rsp.msg == "no match") {
            displayTwoWords("No", "Match", EnrollModeHeader);
            delay(1000);
          }
        }
      }

      // ask if new user should be made a master
      while (mode == "enroll") {
        displayOption("Master??");  // ask if the new user should be made a master
        char *option = choice();
        if (option == "yes") {
          user.notMaster = 0;
          debugf("%s is master", user.name);
          displayTwoWords("Master", user.name, EnrollModeHeader);
          delay(1000);
          break;
        } else if (option == "no") {
          user.notMaster = 1;
          displayTwoWords("Not", "Master", EnrollModeHeader);
          delay(1000);
          break;
        }
      }
    }

    // enroll new user's fingerprint
    while (mode == "enroll") {
      if (exit()) { break; }  // a function to allow user to exit this menu
      uint8_t success = getFingerprintEnroll(current_id);
      debugf("enrollment result %i\n", success);
      if (success == 1) {
        saveUser(user);
        enrolledUsers++;
        Serial.println("saved");
        break;
      }
      debugln("failed to enroll");
      delay(2000);  // replace with millis() later
    }

    while (mode == "enroll") {
      if (exit()) { break; }  // a function to allow user to exit this menu
      if (mode != "enroll") {
        break;
      }
      //Serial.println("ENROLL AGAIN? Y/N");
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

  /////////////////////////////////////// Access Mode //////////////////////////////////
  if (mode == "access") {
    while (1) {
      if (exit()) { break; }                                 // a function to allow user to exit this menu
      displayTwoWords("Place", "Finger", AccessModeHeader);  // replace with finger image or animation
      SensorRsp rsp = getFingerprintID();
      if (!rsp.isError) {
        debugf("user with id %s unlocked the door\n", rsp.msg);
        char *endptr;
        // Convert base 10 string to long, then cast to int
        uint16_t id = (uint16_t)strtol(rsp.msg, &endptr, 10);
        UserRecord user = getUser(id);
        displayTwoWords("Welcome", user.name, AccessModeHeader);
        unlockDoor();
        delay(5000);
        lockDoor();
      } else {
        debugf("sensor err: %s\n", rsp.msg);
        if (rsp.msg == "no match") {
          displayWord("Invalid", AccessModeHeader);
          delay(1000);
        }
      }
    }
  }


  if (mode == "delete") {
    // scroll mode
    // text input
    UserRecord user;
    int16_t startID = 1;
    while (1) {
      if (exit()) { break; }
      displayCustomMenu("Query Opt", "U-Scroll", "D-Name");
      if (UP_BUTTON.wasPressed()) {
        debugln("Scroll mode");
        //////////////// Scroll Mode ////////////////////
        // the user navigates through all users sequentialy
        while (1) {
          if (exit()) { break; }
          user = getUser(startID);
          char buffer[9];
          snprintf(buffer, sizeof(buffer), "Del %s", user.name);
          const char *name = buffer;
          displayOption(name, "exit>");

          if (UP_BUTTON.wasPressed()) {
            debugf("enrolled users: %i\n",enrolledUsers);
            if (enrolledUsers < startID){
              startID = enrolledUsers;
              debugln("user limit exceeded");
            }
            startID++;
          }

          else if (DOWN_BUTTON.wasPressed() ) {
            debugf("enrolled users: %i",enrolledUsers);
            if (startID < 1){
              startID=0;
            }
            startID--;
          } else if (LEFT_BUTTON.wasPressed()) {
            // this button is selected, yes. Yes as in delete the user
            deleteFingerprint(user.id);
            deleteUser(user);
            displayTwoWords(user.name, "deltd", DeleteModeHeader);
            delay(1000);
          }
        }
      } else if (DOWN_BUTTON.wasPressed()) {

        while (1) {
          if (exit()) { break; }
        }
      }
    }
  }

  while (mode == "log") {
    if (exit()) { break; }
    Serial.println("hello log");
  }

  while (mode == "locked") {
    if (exit()) { break; }
    Serial.println("hello locked");
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
  digitalWrite(DOOR_PIN, 0);  //lock the door
}

void unlockDoor() {
  digitalWrite(DOOR_PIN, 1);  //lock the door
}