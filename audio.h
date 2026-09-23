#pragma once

#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>

#ifndef SPEAKER_PIN
#define SPEAKER_PIN PA15
#endif

void audioInit(uint8_t pin = SPEAKER_PIN);
void playTone(uint16_t frequency, uint16_t durationMs);
void stopTone();

void audioBeep();
void audioAccessGranted();
void audioAccessDenied();
void audioEnrollStep();
void audioEnrollSuccess();
void audioEnrollError();
void audioStartup();
void audioDelete();
void audioWarning();

#endif
