#include <Arduino.h>
#include "buttons.h"

// #define UP_BUTTON PB12
// #define DOWN_BUTTON PB13
// #define LEFT_BUTTON PB14
// #define RIGHT_BUTTON PB15

navButton UP_BUTTON(PB12);
navButton DOWN_BUTTON(PB13);
navButton LEFT_BUTTON(PB14);
navButton RIGHT_BUTTON(PB15);


void buttonConfig() {

  UP_BUTTON.begin<UP_BUTTON>();
  DOWN_BUTTON.begin<DOWN_BUTTON>();
  LEFT_BUTTON.begin<LEFT_BUTTON>();
  RIGHT_BUTTON.begin<RIGHT_BUTTON>();
}

char* choice() {
  if (LEFT_BUTTON.wasPressed()) {
    return "yes";
  } else if (RIGHT_BUTTON.wasPressed()) {
    return "no";
  }
  return "";
}
