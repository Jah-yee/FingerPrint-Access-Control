#pragma once

#ifndef BUTTONS_H
#define BUTTONS_H

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

struct navButton {
private:
  volatile bool state = false;  // 0,1,2
  uint8_t pin;
  void stateCheck() {
    state = (digitalRead(pin) == LOW);
  }
  
public:
  //constructor
  navButton(uint8_t buttonPin)
    : pin(buttonPin){};

  template<navButton& ButtonInstance>
  void begin(uint8_t mode = INPUT_PULLUP, PinStatus interruptMode = CHANGE) {
    pinMode(pin, mode);
    attachInterrupt(digitalPinToInterrupt(pin), isr<ButtonInstance>, interruptMode);
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
extern navButton UP_BUTTON;
extern navButton DOWN_BUTTON;
extern navButton LEFT_BUTTON;
extern navButton RIGHT_BUTTON;


void buttonConfig();
uint8_t getButtonsState();

#endif