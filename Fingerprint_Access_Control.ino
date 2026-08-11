#include "icons.h"
#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>


#define UP_BUTTON PB12
#define DOWN_BUTTON PB13
#define LEFT_BUTTON PB14
#define RIGHT_BUTTON PB15

#define AS608_TX PA2
#define AS608_RX PA3

#define SPEAKER_PIN PA11
#define DOOR_PIN PA12

#define MAX_USERS 300

#define USART1

struct UserRecord {
  uint16_t id;    // Fingerprint ID from sensor (e.g., 1 to 50) also serve as eeprom address
  char name[20];  // User name (e.g., "Alex")
  bool isMaster;
  bool active;
};

Uart mySerial(AS608_RX,AS608_TX); //rxtx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//const char modes[4] = {"access","enroll","delete","log"};
char *mode = "access"; // start with access control mode
bool doorLocked = true; // door is locked initially.
bool UserExist = false

uint8_t readnumber(void);
int writeName(); // handle name input
bool getUser(uint16_t userID, UserRecord &user);
int getEnrolledCount();
bool hasEnrolledUsers();
int getEEPROMAddress(uint16_t userID);


uint8_t id;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DOOR_PIN,OUTPUT); 

  for (int i = 27; i <= 30; i++) {
    pinMode(i, INPUT_PULLUP);  // set all button pins as input
  }

  digitalWrite(DOOR_PIN,0); //lock the door
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
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
// show lock icon

// check if any user exist
  // if yes set mode to enroll
  if(hasEnrolledUsers()){
    mode = "enroll";
  }
  // else  mode remains  access
}

void loop() {
  // put your main code here, to run repeatedly:

  while(mode=="enroll"){
    // get number of available records
    int current_id = 1+ getEnrolledCount();
      // if > 299 print no space icon
    if(current_id>MAX_USERS){
      Serial.println("no space for new enrollment");
      mode = "access";
      break;
    }
  
    // do enroll stuff
    Serial.println("Ready to enroll a fingerprint!");
      // ask for name and input name


    // if no records then make the person master during enroll
    // else ask if the person should be made master
    // save details to sensor and eeprom
    //exit enrolling using left button
  }

  while(mode=="access"){
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

  while(mode=="delete"){
    //int id=1;
    // get total records

    // when right button is pressed increment Id and show corresponding record
    // when left button
    // id cannot be less than 1
    // id cannot be greater than number of records
    // exit button

  }

  while(mode =="log"){
    // check if master if not locked the device
    // just show log icon
    // if cmd is get log send log, delete from mem and exit
    // if exit is pressed break
  }

  while(mode == "locked"){
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
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

int getEnrolledCount() {
  int count = 0;
  UserRecord user;
  
  for (int id = 1; id <= MAX_USERS; id++) {
    if (getUser(id, user)) { // Returns user.active
      count++;
    }
  }
  return count;
}


bool hasEnrolledUsers() {
  UserRecord user;
  
  for (int id = 1; id <= MAX_USERS; id++) {
    if (getUser(id, user)) {
      return true; // Found at least one valid enrolled user
    }
  }
  return false; // Storage is completely empty
}

bool getUser(uint16_t userID, UserRecord &user) {
  int addr = getEEPROMAddress(userID);
  EEPROM.get(addr, user);
  return user.active; // Returns false if slot is unassigned
}

int getEEPROMAddress(uint16_t userID) {
  return (userID - 1) * sizeof(UserRecord);
  // each user takes 24bytes, so the next
  // user will be 24 bytes away from the 
  // previous user
}
/*
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
