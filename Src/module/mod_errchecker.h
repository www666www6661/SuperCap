#pragma once
#include "mod_samplemanager.h"
#include <stdint.h>

#define ERROR_UNDER_VOLTAGE 0b00000001
#define ERROR_OVER_VOLTAGE 0b00000010
#define ERROR_BUCK_BOOST 0b00000100
#define ERROR_SHORT_CIRCUIT 0b00001000
#define ERROR_HIGH_TEMPERATURE 0b00010000
#define ERROR_NO_POWER_INPUT 0b00100000
#define ERROR_CAPACITOR 0b01000000

typedef struct {
  uint8_t currentError;
  uint16_t shortCircuitCnt;
  uint16_t restartCoolDown;
  uint16_t errorCoolDown;
} Module_ErrChecker_Data;

extern Module_ErrChecker_Data module_errchecker_data;

void Module_ErrChecker_Init();
void Module_ErrChecker_ShortCircuit();
