#include "api/Common.h"
#include <EEPROM.h>
#include "buttons.h"
#include "users.h"
#include "display.h"
#include "debugMacro.h"

int8_t cursorPos = 0;
int enrolledUsers = 0;
const int TOTAL_STORAGE_USERS = 100;
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
  int addr = getEEPROMAddress(user.id); // todo: check if spot is already taken, but this is unlikely to happen
  EEPROM.put(addr, user);  // Writes directly to internal flash emulation

  Serial.print("Saved user ");
  Serial.print(user.name);
  Serial.print(" at address ");
  Serial.println(addr);
}

void deleteUser(UserRecord &user){
user.magic = 0; // this will mark that area in memory as overwritten
saveUser(user); // funny enough we still have to save, so as to update user.magic in memory
/*
due to the sequential nature of saving users with incremental id, anytime deletion occurs 
there would be a gap in memory. To resolve this, we will update that gap with the latest
user
*/ 
UserRecord latestUser = getUser(enrolledUsers);
latestUser.id=user.id;
saveUser(latestUser);
}

void deleteAll(){
 
  for(int i=0;i<TOTAL_STORAGE_USERS;i++){
  EEPROM.update(i,0);
   i= map(i, 0, TOTAL_STORAGE_USERS, 0, 100);
  debugf("Deleting user: %i%%\n",i);
  }
  debugln("all users deleted");

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