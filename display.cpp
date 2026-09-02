#include <Arduino.h>
//#include "icons.h"
#include "display.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool checkDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    return false;
  }
  return true;
}

// void displayLogo(){
//   display.setRotation(0);
//   display.clearDisplay();
//   display.display();

//   display.clearDisplay();
//   display.drawBitmap(0, 0, kriitor, 100, 100, SSD1306_WHITE);
//   display.display();
// }

void displayTwoWords(char* word1, char* word2) {
  display.clearDisplay();
  display.setTextSize(3);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0,10);  
  display.println(word1);   
  display.setCursor(0,40);  
  display.println(word2);
  display.display();
}

void displayWord(char* word){
    display.clearDisplay();
  display.setTextSize(3);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(30,30);  
  display.println(word);   
  display.display();
}

void displayMenu(){
  display.clearDisplay();
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
    display.setCursor(0,0);
  display.println("U - Enroll");
  display.println("D - Access");
  display.println("L - Delete");
  display.println("R - Log");
  display.display();
}

void displayTextInput(char *name, int8_t cursorPos) {
  display.clearDisplay();
  display.setTextSize(3);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 20);
  display.print(name[0]);
    if(cursorPos==0){
    display.setCursor(0, 50);
    display.print("*");
    }

  display.setCursor(30, 20);
  display.print(name[1]);
      if(cursorPos==1){
    display.setCursor(30, 50);
    display.print("*");
    }
  display.setCursor(60, 20);
  display.print(name[2]);
        if(cursorPos==2){
    display.setCursor(60, 50);
    display.print("*");
    }
  display.setCursor(90, 20);
  display.print(name[3]);
        if(cursorPos==3){
    display.setCursor(90, 50);
    display.print("*");
    }
  display.display();
}
