#include "Arduino.h"
#include <Adafruit_Fingerprint.h>
#include "buttons.h"
#include "biometrics.h"

#define AS608_TX PA2
#define AS608_RX PA3
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

#define USART1

Uart mySerial(AS608_RX, AS608_TX);  //rxtx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void biometricsSetup() {
  Serial.println("\n\nFingerprint sensor enrollment");

  finger.begin(57600);
while(1){
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    break;
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    // while (1) { delay(1); }
    debugln("Trying again");
    delay(1000);
  }
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

// uint8_t readnumber(void) {
//   uint8_t num = 0;

//   while (num == 0) {
//     while (!Serial.available())
//       ;
//     num = Serial.parseInt();
//   }
//   return num;
// }

uint8_t getFingerprintEnroll(uint8_t id) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  displayTwoWords("PLACE", "FINGER","Finger Enroll");
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
      displayTwoWords("too", "messy","Finger Enroll");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
    default:
      Serial.println("Unknown error");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
  }

  Serial.println("Remove finger");
  displayTwoWords("REMOVE", "FINGER","Finger Enroll");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  displayTwoWords("place", "finger","Finger Enroll");
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
      displayTwoWords("clean", "fing","Finger Enroll");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      displayWord("error","Finger Enroll");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      displayWord("error","Finger Enroll");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      displayWord("error","Finger Enroll");
      return p;
    default:
      Serial.println("Unknown error");
      displayWord("error","Finger Enroll");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    displayTwoWords("PRINTS", "MATCHED","Finger Enroll");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    displayTwoWords("COMMS", "ERROR","Finger Enroll");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    displayTwoWords("NO", "MATCH","Finger Enroll");
    return p;
  } else {
    Serial.println("Unknown error");
    displayWord("Error","Finger Enroll");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    displayWord("Saved","Finger Enroll");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    displayWord("Error","Finger Enroll");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    displayWord("Error","Finger Enroll");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    displayWord("Error","Finger Enroll");
    return p;
  } else {
    Serial.println("Unknown error");
    displayWord("Error","Finger Enroll");
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
      rsp.msg = "No finger";
      rsp.isError = true;
      return rsp;
    case FINGERPRINT_PACKETRECIEVEERR:
      rsp.msg = "comms err";
      rsp.isError = true;
      return rsp;
    case FINGERPRINT_IMAGEFAIL:
      rsp.msg = "img err";
      rsp.isError = true;
      return rsp;
    default:
      rsp.msg = "error";
      rsp.isError = true;
      return rsp;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      rsp.msg = "messy";
      rsp.isError = true;
      return rsp;
    case FINGERPRINT_PACKETRECIEVEERR:
      rsp.msg = "comms err";
      rsp.isError = true;
      return rsp;
    case FINGERPRINT_FEATUREFAIL:
      rsp.msg = "can't find features";
      rsp.isError = true;
      return rsp;
    case FINGERPRINT_INVALIDIMAGE:
      rsp.msg = "invalid image";
      rsp.isError = true;
      return rsp;
    default:
      rsp.msg = "err";
      rsp.isError = true;
      return rsp;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");

  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    rsp.msg = "comms err";
    rsp.isError = true;
    return rsp;
  } else if (p == FINGERPRINT_NOTFOUND) {
    rsp.msg = "no match";
    rsp.isError = true;
    return rsp;
  } else {
    rsp.msg = "err";
    rsp.isError = true;
    return rsp;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  static char idBuffer[12];
  itoa(finger.fingerID, idBuffer, 10);
  rsp.msg = idBuffer;
  rsp.isError = false;
  return rsp;
}
