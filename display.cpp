#include <Arduino.h>
#include "display.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool checkDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    return false;
  }
  return true;
}

void drawFrame(const char *title, const char *optionR="", const char *optionL="") {
  //display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  const int16_t lineY = 15;
  display.drawLine(0, lineY, 128, lineY, SSD1306_WHITE);
  display.setTextSize(2);
  //center text
  int16_t x1, y1;
  uint16_t width, height;
  display.getTextBounds(title, 0, 0, &x1, &y1, &width, &height);
  int x = ((SCREEN_WIDTH - width) / 2);
  int y = ((SCREEN_HEIGHT - height) / 2);
  display.setCursor(x, 0);
  display.setTextWrap(false);
  display.print(title);
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.print(optionR);
  display.setCursor(100, 50);
  display.print(optionL);
}

void displayTwoWords(const char *word1, const char *word2, const char *frameTitle, const char* optionR, const char* optionL) {
  display.clearDisplay();
  drawFrame(frameTitle, optionR, optionL);
  display.setTextWrap(0);
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 20);
  display.println(word1);
  display.setCursor(50, 40);
  display.println(word2);
  display.display();
}

void displayWord(const char *word,const char *frameTitle, const char* optionR,const char* optionL) {
  display.clearDisplay();
  drawFrame(frameTitle, optionR, optionL);
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(30, 30);
  display.println(word);
  display.display();
}

void displayMenu() {
  display.clearDisplay();
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);
  display.println("U - Enroll");
  display.println("D - Access");
  display.println("L - Delete");
  display.println("R - Log");
  display.display();
}


void displayTextInput(char *name, int8_t cursorPos) {
  display.clearDisplay();
  const int16_t yPointerPos = 40;
  const int16_t yNamePos = 20;

  const int16_t xName0Pos = 0;
  const int16_t xName1Pos = 30;
  const int16_t xName2Pos = 60;
  const int16_t xName3Pos = 90;
  drawFrame("Enter Name", "<YES", "NO>");
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(xName0Pos, yNamePos);
  display.print(name[0]);
  if (cursorPos == 0) {
    display.setTextSize(1);
    display.setCursor(xName0Pos, yPointerPos);
    display.print("*");
  }
  display.setTextSize(2);
  display.setCursor(xName1Pos, yNamePos);
  display.print(name[1]);
  if (cursorPos == 1) {
    display.setTextSize(1);
    display.setCursor(xName1Pos, yPointerPos);
    display.print("*");
  }
  display.setTextSize(2);
  display.setCursor(xName2Pos, yNamePos);
  display.print(name[2]);
  if (cursorPos == 2) {
    display.setTextSize(1);
    display.setCursor(xName2Pos, yPointerPos);
    display.print("*");
  }
  display.setTextSize(2);
  display.setCursor(xName3Pos, yNamePos);
  display.print(name[3]);
  if (cursorPos == 3) {
    display.setTextSize(1);
    display.setCursor(xName3Pos, yPointerPos);
    display.print("*");
  }
  display.display();
}

void displayOption(const char *text, const char* optionR, const char* optionL) {
  display.clearDisplay();
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(20, 10);
  display.println(text);
  display.setCursor(0, 50);
  display.print(optionL);
  display.setCursor(100, 50);
  display.print(optionR);
  display.display();
}

void displayCustomMenu(const char *title, const char *option1, const char *option2) {
  display.clearDisplay();
  drawFrame(title);
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 25);
  display.println(option1);
  display.setCursor(0, 49);
  display.println(option2);
  display.display();
}

void display_three_words(const char* text1, const char* text2, const char* text3, int size) {
  display.clearDisplay();
  display.setTextSize(size);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);
  display.println(text1);
  display.setCursor(40, 25);
  display.println(text2);
  display.setCursor(0, 50);
  display.println(text3);
  display.display();
 
}