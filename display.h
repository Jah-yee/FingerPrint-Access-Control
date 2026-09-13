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
void displayWord(const char *word,const char *frameTitle, const char* optionR="",const char* optionL="");
void displayTwoWords(const char *word1, const char *word2, const char *frameTitle, const char* optionR="", const char* optionL="");
void displayTextInput(char *name, int8_t cursorPos);
void displayOption(const char *text, const char* optionR="NO>", const char* optionL="<YES");
void displayCustomMenu(const char *title, const char *option1, const char *option2);
void displayMenu();
void display_three_words(const char* text1, const char* text2, const char* text3, int size);
#endif