#pragma once

#include "mod_powerctrl.h"
#include "mod_samplemanager.h"

typedef struct {
  Module_SampleManager_Param sampler;

} SuperCap_Param;

typedef struct {
  Module_SampleManager sampler_;
  Module_SampleManager_Param param_;
  Module_PowerCtrl powerctrl_;
} SuperCap;

// extern SuperCap supercap;

void SuperCap_Start();
