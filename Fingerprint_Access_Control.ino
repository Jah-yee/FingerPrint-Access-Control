// clean up
// when there is an error it returns and it still saves. I have to prevent that
// tell them to be cleaning the sensor
#include "icons.h"
#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>

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


#define UP_BUTTON PB12
#define DOWN_BUTTON PB13
#define LEFT_BUTTON PB14
#define RIGHT_BUTTON PB15

#define AS608_TX PA2
#define AS608_RX PA3

#define SPEAKER_PIN PA15
#define DOOR_PIN PC13

#define MAX_USERS 200

#define USART1

struct UserRecord {
  uint8_t magic;          // 1 byte
  uint16_t id;           // 2 bytesFingerprint ID from sensor (e.g., 1 to 50) also serve as eeprom address
  char name[5];         // 5 bytes User name (e.g., "Alex")
  uint8_t notMaster=1; //1 byte: 0 means user is a master
};

Uart mySerial(AS608_RX, AS608_TX);  //rxtx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//const char modes[4] = {"access","enroll","delete","log"};
char *mode = "access";   // start with access control mode
bool doorLocked = true;  // door is locked initially.
bool UsersExists = false;
int enrolledUsers = 0;

uint8_t readnumber(void);
int writeName();  // handle name input
bool userExist(uint16_t userID, UserRecord &user);
int getEnrolledCount();
int getEEPROMAddress(uint16_t userID);
uint8_t getFingerprintEnroll();
void saveUser(uint16_t userID, const char *name, uint8_t level);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);


  delay(5000);
// it takes almost a second to overwrite a memory block, so I will just change the value of magic
// update is faster, but takes too long, so I will just update magic
//  debugln("clearing eeprom");
//       for (int i = 0 ; i < EEPROM.length() ; i++) {
//     EEPROM.update(i, 0xFF);
//     debugf("cleared: %i\n",i);
//   }
  
  Serial.println("starting");
  Serial.printf("EEPROM size: %i\n", EEPROM.length());
  pinMode(DOOR_PIN, OUTPUT);

  for (int i = 27; i <= 30; i++) {
    pinMode(i, INPUT_PULLUP);  // set all button pins as input
  }

  digitalWrite(DOOR_PIN, 0);  //lock the door
                              // show loading animation with my logo
                              // play welcome sound

  // adafruit finger print set up functions
  delay(100);
  Serial.println("\n\nFingerprint sensor enrollment");

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
  // show lock icon

  // check if any user exist
  // if yes set mode to enroll
  enrolledUsers = getEnrolledCount();
  if (enrolledUsers == 0) {
    mode = "enroll";
    Serial.println("No users found. Entering enroll mode.");
  }

  // I limited max users to 200 because of the function above.
  // if

  // else  mode remains as access
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("hello loop");
  while (mode == "enroll") {
    UserRecord user;
    // get number of available records
    int current_id = 1 + enrolledUsers;
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
      user.notMaster = 0; // make master
      Serial.println("user is a master");
    }
    // else ask if the person should be made master
    else {
      Serial.println("Master?");
    }
    while(1){
    bool success = !getFingerprintEnroll(current_id);
    //while (!getFingerprintEnroll(current_id));
    if(success){
      saveUser(user);
      enrolledUsers++;
      break;
    }
    debugln("failed to enroll");
    delay(2000); // replace with millis() later
    }
    // save details to sensor and eeprom
    //exit enrolling using left button
    // mode="access";
    // break;
  }

  while (mode == "access") {
    Serial.println("hello access");
    delay(1000);
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

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
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

uint8_t getFingerprintEnroll(uint8_t id) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
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
