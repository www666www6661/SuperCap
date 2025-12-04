#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool outputEnabled;
  uint8_t errorCode;
  bool lowBattery;

  float capEnergy;
  float chassisPower;
  float chassisPowerLimit;

  float realVBToVA;
  // uint16_t communicationTimeoutCnt;
} Module_Status;

void Module_Status_Init(Module_Status *this);
