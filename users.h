#pragma once

#ifndef USERS_H
#define USERS_H

  //EEPROM.update(0, 0xFF);
#define MAX_USERS 300

struct UserRecord {
  uint8_t magic;                          // 1 byte
  uint16_t id;                             // 1 bytes Fingerprint ID from sensor (e.g., 1 to 50) also serve as eeprom address
  char name[5] = { '@', '@', '@', '@' };  // 5 bytes User name (e.g., "Alex")
  uint8_t notMaster = 1;                  //1 byte: 0 means user is a master
};

extern int enrolledUsers;

void getUserName(char (&name)[5]);
int getEnrolledCount();
int getEEPROMAddress(uint16_t userID);
void saveUser(UserRecord &user);
bool userExist(uint16_t userID, UserRecord &user);
UserRecord getUser(uint16_t id);
uint16_t getNextAvailableID();
int getMasterCount();
void deleteUser(UserRecord &user);
void deleteAll();
#endif