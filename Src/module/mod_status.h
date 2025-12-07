#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float capEnergy;
  float chassisPower;
  float chassisPowerLimit;

  float realVBToVA;

  bool outputEnabled;
  bool lowBattery;
  uint8_t errorcode;
} Module_Status;
