#include <Arduino.h>
#include "buttons.h"
#include "audio.h"

// #define UP_BUTTON PB12
// #define DOWN_BUTTON PB13
// #define LEFT_BUTTON PB14
// #define RIGHT_BUTTON PB15

Button UP_BUTTON(PB12);
Button DOWN_BUTTON(PB13);
Button LEFT_BUTTON(PB14);
Button RIGHT_BUTTON(PB15);


void buttonConfig() {

  UP_BUTTON.begin();
  DOWN_BUTTON.begin();
  LEFT_BUTTON.begin();
  RIGHT_BUTTON.begin();
}

char* choice() {
  if (LEFT_BUTTON.pressed()) {
    audioBeep();
    return "yes";
  } else if (RIGHT_BUTTON.pressed()) {
    audioBeep();
    return "no";
  }
  return "";
}

