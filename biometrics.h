#pragma once

#include <Arduino.h>
#include "display.h"

struct SensorRsp {
  char *msg;
  bool isError;
};

void biometricsSetup();
uint8_t readnumber(void);
uint8_t getFingerprintEnroll(uint8_t id);
SensorRsp getFingerprintID();
bool deleteFingerprint(uint16_t id);
void deleteAllPrints();
