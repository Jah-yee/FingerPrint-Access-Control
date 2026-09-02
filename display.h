#pragma once

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDR 0x3C

extern Adafruit_SSD1306 display;

bool checkDisplay();
void displayLogo();
void displayWord(char* word);
void displayTwoWords(char* word1, char* word2);
void displayTextInput(char *name, int8_t cursorPos);
void displayMenu();

#endif