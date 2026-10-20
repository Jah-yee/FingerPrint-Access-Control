#include <Arduino.h>
#include "door.h"

void lockDoor() {
  digitalWrite(DOOR_PIN, 0);  // lock the door
}

void unlockDoor() {
  digitalWrite(DOOR_PIN, 1);  // unlock the door
}