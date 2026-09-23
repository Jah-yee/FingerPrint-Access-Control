#pragma once
#include <Button.h>
#ifndef BUTTONS_H
#define BUTTONS_H



struct navButton {
private:
  volatile bool state = false;
  volatile uint32_t lastInterruptTime = 0;
  uint8_t pin;
  void stateCheck() {
    uint32_t now = millis();
    if (now - lastInterruptTime > 10) {  // 50ms debounce window
      state = (digitalRead(pin) == LOW);
      lastInterruptTime = now;
    }
  }

public:
  //constructor
  navButton(uint8_t buttonPin)
    : pin(buttonPin){};

  template<navButton& ButtonInstance>
  void begin(uint8_t mode = INPUT_PULLUP, uint32_t interruptMode = CHANGE) {
    pinMode(pin, mode);
    attachInterrupt(digitalPinToInterrupt(pin), isr<ButtonInstance>, (PinStatus)interruptMode);
  }
  bool wasPressed() {
    if (state) {
      state = false;
      return true;
    }
    return false;
  }

  // Static ISR template wrapper
  template<navButton& ButtonInstance>
  static void isr() {
    ButtonInstance.stateCheck();
  }
};
extern Button UP_BUTTON;
extern Button DOWN_BUTTON;
extern Button LEFT_BUTTON;
extern Button RIGHT_BUTTON;


void buttonConfig();
char* choice();
#endif