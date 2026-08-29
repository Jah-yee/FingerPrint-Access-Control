#pragma once

#ifndef BUTTONS_H
#define BUTTONS_H

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

struct navButton {
  volatile uint8_t state = 2;  // 0,1,2
  uint8_t pin;

  //constructor
  navButton(uint8_t buttonPin) :pin(buttonPin) {};

  void begin(uint8_t mode = INPUT_PULLUP) {
    pinMode(pin, mode);
  }
  
  void stateCheck() {
    if (digitalRead(pin) == LOW) {
      // The button was just pressed (Falling edge)
      state = 1;
    } else {
      // The button was just released (Rising edge)
      state = 0;
    }
  }

uint8_t getButtonsState();

#endif