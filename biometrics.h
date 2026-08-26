#pragma once

#ifndef BIOMETRICS_H
#define BIOMETRICS_H

#include <Arduino.h>
#include "display.h"

struct SensorRsp {
  uint8_t id;
  bool isError;
};

void biometricsSetup();
uint8_t readnumber(void);
uint8_t getFingerprintEnroll(uint8_t id);
SensorRsp getFingerprintID();

#endif