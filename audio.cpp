#include "audio.h"

static uint8_t audioPin = SPEAKER_PIN;

void audioInit(uint8_t pin) {
  audioPin = pin;
  pinMode(audioPin, OUTPUT);
  digitalWrite(audioPin, LOW);
  noTone(audioPin);
}

void playTone(uint16_t frequency, uint16_t durationMs) {
  tone(audioPin, frequency, durationMs);
  delay(durationMs);
  noTone(audioPin);
}

void stopTone() {
  noTone(audioPin);
  digitalWrite(audioPin, LOW);
}

void audioBeep() {
  tone(audioPin, 2400, 35);
  delay(40);
  noTone(audioPin);
}

void audioAccessGranted() {
  tone(audioPin, 1046, 100);
  delay(110);
  tone(audioPin, 1318, 100);
  delay(110);
  tone(audioPin, 1568, 180);
  delay(190);
  noTone(audioPin);
}

void audioAccessDenied() {
  tone(audioPin, 300, 150);
  delay(170);
  tone(audioPin, 220, 250);
  delay(260);
  noTone(audioPin);
}

void audioEnrollStep() {
  tone(audioPin, 1500, 60);
  delay(70);
  noTone(audioPin);
}

void audioEnrollSuccess() {
  tone(audioPin, 1000, 80);
  delay(90);
  tone(audioPin, 1500, 80);
  delay(90);
  tone(audioPin, 2000, 160);
  delay(170);
  noTone(audioPin);
}

void audioEnrollError() {
  tone(audioPin, 400, 120);
  delay(140);
  tone(audioPin, 250, 220);
  delay(240);
  noTone(audioPin);
}

void audioStartup() {
  tone(audioPin, 800, 60);
  delay(75);
  tone(audioPin, 1200, 60);
  delay(75);
  tone(audioPin, 1600, 100);
  delay(110);
  noTone(audioPin);
}

void audioDelete() {
  tone(audioPin, 600, 100);
  delay(110);
  tone(audioPin, 400, 150);
  delay(160);
  noTone(audioPin);
}

void audioWarning() {
  tone(audioPin, 1800, 60);
  delay(80);
  tone(audioPin, 1800, 60);
  delay(80);
  noTone(audioPin);
}
