#pragma once

#include "mod_errchecker.h"
#include "mod_powerctrl.h"
#include "mod_samplemanager.h"

typedef struct {
  Module_SampleManager_Param sampler;
  Module_PowerCtrl_Param powerctrl;
  Module_ErrChecker_Param errchk;

} SuperCap_Param;

typedef struct {
  Module_Status status_;
  Module_SampleManager sampler_;
  Module_PowerCtrl powerctrl_;
  Module_ErrChecker errchk_;
} SuperCap;

// extern SuperCap supercap;

void SuperCap_Start(void);
void SuperCap_control(void);
void HRTIM1_Master_IRQHandler(void);
