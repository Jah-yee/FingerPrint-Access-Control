#include "Arduino.h"
#include <Adafruit_Fingerprint.h>
#include "buttons.h"
#include "biometrics.h"

#define AS608_TX PA2
#define AS608_RX PA3

#define USART1

Uart mySerial(AS608_RX, AS608_TX);  //rxtx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void biometricsSetup(){
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

uint8_t getFingerprintEnroll(uint8_t id) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
   displayTwoWords("PLACE","FINGER");
  while (p != FINGERPRINT_OK) {
    uint8_t button = getButtonsState();
    if(button == RIGHT){
     // debugln("cancelled");
      return 0xAA;
    }
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
      displayTwoWords("too", "messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      displayWord("error"); // replace with icon
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      displayWord("error"); // replace with icon
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      displayWord("error"); // replace with icon
      return p;
    default:
      Serial.println("Unknown error");
      displayWord("error"); // replace with icon
      return p;
  }

  Serial.println("Remove finger");
  displayTwoWords("REMOVE","FINGER");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  displayTwoWords("place", "finger");
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
      displayTwoWords("clean", "fing");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      displayWord("error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      displayWord("error");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      displayWord("error");
      return p;
    default:
      Serial.println("Unknown error");
      displayWord("error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    displayTwoWords("PRINTS", "MATCHED");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    displayTwoWords("COMMS","ERROR");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    displayTwoWords("NO","MATCH");
    return p;
  } else {
    Serial.println("Unknown error");
    displayWord("Error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    displayWord("Saved");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
       displayWord("Error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
       displayWord("Error");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
       displayWord("Error");
    return p;
  } else {
    Serial.println("Unknown error");
       displayWord("Error");
    return p;
  }

  return true;
}

SensorRsp getFingerprintID() {
  SensorRsp rsp;
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      rsp.id=p; rsp.isError=true;
      return rsp;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
            rsp.id=p; rsp.isError=true;
      return rsp;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
            rsp.id=p; rsp.isError=true;
      return rsp;
    default:
      Serial.println("Unknown error");
            rsp.id=p; rsp.isError=true;
      return rsp;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
            rsp.id=p; rsp.isError=true;
      return rsp;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
            rsp.id=p; rsp.isError=true;
      return rsp;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
            rsp.id=p; rsp.isError=true;
      return rsp;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
            rsp.id=p; rsp.isError=true;
      return rsp;
    default:
      Serial.println("Unknown error");
            rsp.id=p; rsp.isError=true;
      return rsp;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
         rsp.id=p; rsp.isError=true;
      return rsp;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
         rsp.id=p; rsp.isError=true;
      return rsp;
  } else {
    Serial.println("Unknown error");
          rsp.id=p; rsp.isError=true;
      return rsp;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
      rsp.id=finger.fingerID; rsp.isError=false;
      return rsp;
}

// // returns -1 if failed, otherwise returns ID #
// int getFingerprintIDez() {
//   uint8_t p = finger.getImage();
//   if (p != FINGERPRINT_OK)  return -1;

//   p = finger.image2Tz();
//   if (p != FINGERPRINT_OK)  return -1;

//   p = finger.fingerFastSearch();
//   if (p != FINGERPRINT_OK)  return -1;

//   // found a match!
//   Serial.print("Found ID #"); Serial.print(finger.fingerID);
//   Serial.print(" with confidence of "); Serial.println(finger.confidence);
//   return finger.fingerID;
// }
