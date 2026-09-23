#include "Arduino.h"
#include <Adafruit_Fingerprint.h>
#include "buttons.h"
#include "biometrics.h"
#include "audio.h"

//#define DEBUG 1
#include "debugMacro.h"

#define AS608_TX PA2
#define AS608_RX PA3



#define USART1

Uart mySerial(AS608_RX, AS608_TX);  //rxtx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void biometricsSetup() {
  debugln("\n\nFingerprint sensor enrollment");

  finger.begin(57600);
while(1){
  if (finger.verifyPassword()) {
    debugln("Found fingerprint sensor!");
    break;
  } else {
    debugln("Did not find fingerprint sensor :(");
    // while (1) { delay(1); }
    debugln("Trying again");
    delay(1000);
  }
}
  debugln(F("Reading sensor parameters"));
  finger.getParameters();
  debug(F("Status: 0x"));
  debugln(finger.status_reg, HEX);
  debug(F("Sys ID: 0x"));
  debugln(finger.system_id, HEX);
  debug(F("Capacity: "));
  debugln(finger.capacity);
  debug(F("Security level: "));
  debugln(finger.security_level);
  debug(F("Device address: "));
  debugln(finger.device_addr, HEX);
  debug(F("Packet len: "));
  debugln(finger.packet_len);
  debug(F("Baud rate: "));
  debugln(finger.baud_rate);
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
  debug("Waiting for valid finger to enroll as #");
  debugln(id);
  displayTwoWords("PLACE", "FINGER","Finger Enroll");
  while (p != FINGERPRINT_OK) {

    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        audioEnrollStep();
        debugln("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        debug(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        debugln("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        debugln("Imaging error");
        break;
      default:
        debugln("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      debugln("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      audioEnrollError();
      debugln("Image too messy");
      displayTwoWords("too", "messy","Finger Enroll");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      audioEnrollError();
      debugln("Communication error");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
    case FINGERPRINT_FEATUREFAIL:
      audioEnrollError();
      debugln("Could not find fingerprint features");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      audioEnrollError();
      debugln("Could not find fingerprint features");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
    default:
      audioEnrollError();
      debugln("Unknown error");
      displayWord("error","Finger Enroll");  // replace with icon
      return p;
  }

  debugln("Remove finger");
  displayTwoWords("REMOVE", "FINGER","Finger Enroll");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  debug("ID ");
  debugln(id);
  p = -1;
  debugln("Place same finger again");
  displayTwoWords("place", "finger","Finger Enroll");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        audioEnrollStep();
        debugln("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        debug(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        debugln("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        debugln("Imaging error");
        break;
      default:
        debugln("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      debugln("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      audioEnrollError();
      debugln("Image too messy");
      displayTwoWords("clean", "fing","Finger Enroll");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      audioEnrollError();
      debugln("Communication error");
      displayWord("error","Finger Enroll");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      audioEnrollError();
      debugln("Could not find fingerprint features");
      displayWord("error","Finger Enroll");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      audioEnrollError();
      debugln("Could not find fingerprint features");
      displayWord("error","Finger Enroll");
      return p;
    default:
      audioEnrollError();
      debugln("Unknown error");
      displayWord("error","Finger Enroll");
      return p;
  }

  // OK converted!
  debug("Creating model for #");
  debugln(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    debugln("Prints matched!");
    displayTwoWords("PRINTS", "MATCHED","Finger Enroll");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    audioEnrollError();
    debugln("Communication error");
    displayTwoWords("COMMS", "ERROR","Finger Enroll");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    audioEnrollError();
    debugln("Fingerprints did not match");
    displayTwoWords("NO", "MATCH","Finger Enroll");
    return p;
  } else {
    audioEnrollError();
    debugln("Unknown error");
    displayWord("Error","Finger Enroll");
    return p;
  }

  debug("ID ");
  debugln(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    audioEnrollSuccess();
    debugln("Stored!");
    displayWord("Saved","Finger Enroll");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    audioEnrollError();
    debugln("Communication error");
    displayWord("Error","Finger Enroll");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    audioEnrollError();
    debugln("Could not store in that location");
    displayWord("Error","Finger Enroll");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    audioEnrollError();
    debugln("Error writing to flash");
    displayWord("Error","Finger Enroll");
    return p;
  } else {
    audioEnrollError();
    debugln("Unknown error");
    displayWord("Error","Finger Enroll");
    return p;
  }

  return FINGERPRINT_OK;
}

SensorRsp getFingerprintID() {
  SensorRsp rsp;
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      debugln("Image taken");
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
      debugln("Image converted");
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
    debugln("Found a print match!");

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
  debug("Found ID #");
  debug(finger.fingerID);
  debug(" with confidence of ");
  debugln(finger.confidence);
  static char idBuffer[12];
  itoa(finger.fingerID, idBuffer, 10);
  rsp.msg = idBuffer;
  rsp.isError = false;
  return rsp;
}

bool deleteFingerprint(uint16_t id) {
  uint16_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    debugln("Deleted!");
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    debugln("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    debugln("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    debugln("Error writing to flash");
  } else {
    debug("Unknown error: 0x"); debugln(p, HEX);
  }

  return false;
}

void deleteAllPrints(){
      finger.emptyDatabase();
    debugln("all print deleted");
}