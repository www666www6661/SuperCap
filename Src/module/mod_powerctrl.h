#pragma once

#include "comp_pid.h"
#include "mod_samplemanager.h"
#include "mod_status.h"
#include <stdbool.h>

#define I_LIMIT 10.0f
#define MAX_CAP_VOLTAGE 25.0f

typedef struct {
  float min_cap_iout;
  float max_cap_iout;
  float cap_v_cutoff;
  float cap_v_normal;

  Component_PID_Param vbside;
  Component_PID_Param iaside;
  Component_PID_Param preferee;
  Component_PID_Param energy;

  float init_referee_power_limit;
  float init_energy_remain;
  float init_output_duty;
  float init_base_referee_power;
} Module_PowerCtrl_Param;

typedef struct {
  float refereePowerLimit;
  float energyRemain;
  bool enableOutput;
} Module_PowerCtrl_ControlData;

typedef struct {
  float outputDuty;
  float baseRefereePower;
  float targetRefereePower;
  float lastRefereePowerLimit;
  float targetAPower;
  float targetIA;
} Module_PowerCtrl_TempData;

typedef struct {
  Component_PID Component_PID_vbside_;
  Component_PID Component_PID_iaside_;
  Component_PID Component_PID_prefree_;
  Component_PID Component_PID_energy_;

  Module_PowerCtrl_ControlData control_data_;
  Module_PowerCtrl_TempData temp_data_;
  Module_PowerCtrl_Param param_;
} Module_PowerCtrl;

void Module_PowerCtrl_Init(Module_PowerCtrl *this,
                           Module_PowerCtrl_Param param);
void Module_PowerCtrl_Update(Module_PowerCtrl *this);
