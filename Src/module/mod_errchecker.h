#pragma once
#include "mod_samplemanager.h"
#include "mod_status.h"
#include <stdint.h>

#define ERROR_UNDER_VOLTAGE 0b00000001
#define ERROR_OVER_VOLTAGE 0b00000010
#define ERROR_BUCK_BOOST 0b00000100
#define ERROR_SHORT_CIRCUIT 0b00001000
#define ERROR_HIGH_TEMPERATURE 0b00010000
#define ERROR_NO_POWER_INPUT 0b00100000
#define ERROR_CAPACITOR 0b01000000

// #define SHORT_CIRCUIT_VOLTAGE 8.0f
// #define SHORT_CIRCUIT_CURRENT 12.0f

typedef struct {
  uint32_t SHORT_CIRCUIT_VOLTAGE;
  uint32_t SHORT_CIRCUIT_CURRENT;

  Module_Status *status_;
  Module_SampleManager *sampler_;

} Module_ErrChecker_Param;

typedef struct {
  uint32_t short_circuit_cnt_;
  uint32_t restart_cooldown_cnt_;
  uint32_t error_cooldown_cnt_;

  Module_ErrChecker_Param param_;
  Module_Status *status_;
  Module_SampleManager *sampler_;
} Module_ErrChecker;

void Module_ErrChecker_Init(Module_ErrChecker *this,
                            Module_ErrChecker_Param param);
void Module_ErrChecker_ShortChk(Module_ErrChecker *this);
