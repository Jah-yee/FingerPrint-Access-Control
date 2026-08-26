#include <Arduino.h>
#include "buttons.h"

#define UP_BUTTON PB12
#define DOWN_BUTTON PB13
#define LEFT_BUTTON PB14
#define RIGHT_BUTTON PB15

uint8_t getButtonsState() {
  if (!digitalRead(LEFT_BUTTON)) {
    return LEFT;
  }

  else if (!digitalRead(RIGHT_BUTTON)) {
    return RIGHT;
  } else {
    return 0;
  }
}