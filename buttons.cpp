#include <Arduino.h>
#include "buttons.h"

#define UP_BUTTON PB12
#define DOWN_BUTTON PB13
#define LEFT_BUTTON PB14
#define RIGHT_BUTTON PB15

navButton RIGHT_BUTTON(PB15)


uint8_t getButtonsState() {
  if (!digitalRead(LEFT_BUTTON)) {
    return LEFT;
  }

  else if (!digitalRead(RIGHT_BUTTON)) {
    return RIGHT;
  } 
  
  else if (!digitalRead(UP_BUTTON)){
    return UP;
  }
  else if(!digitalRead(DOWN_BUTTON)){
    return DOWN;
  }
  else {
    return 0;
  }
  delay(500); // play loading animation
}