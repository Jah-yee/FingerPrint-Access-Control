#include <EEPROM.h>
#include "buttons.h"
#include "users.h"
#include "display.h"

int8_t cursorPos = 0;
#define VALID_MAGIC 0xA5  // to know if a memory sector is availabe for overwriting

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