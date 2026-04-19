#pragma once

#include "comp_pid.h"
#include "dev_buckboost.h"
#include "mod_samplemanager.h"
#include "mod_status.h"

typedef struct
{
    float dt;

    Module_Status *status_;
    Module_SampleManager *sampler_;

    float default_energy;
    float default_output_duty;
    float default_base_referee_power;

    float share_gain;
    float share_limit;

    Component_PID_Param vbside;
    Component_PID_Param ialpha;
    Component_PID_Param ibeta;
    Component_PID_Param igamma;
    Component_PID_Param preferee;
    Component_PID_Param energy;

    Device_BuckBoost_Param buckboost;

} Module_PowerCtrl_Param;

typedef struct
{
    float dt;
    float base_referee_power_;

    float output_duty_;
    float pRefree_setpoint_;
    float paside_setpoint_;
    float iaside_setpoint_;
    float iphase_setpoint_;

    uint32_t communication_timeout_cnt_;

    Device_BuckBoostMode_t buckboost_mode_;

    Component_PID PID_vbside_;
    Component_PID PID_ialpha_;
    Component_PID PID_ibeta_;
    Component_PID PID_igamma_;
    Component_PID PID_pRefree_;
    Component_PID PID_energy_;

    Device_BuckBoost buckboost_alpha_;
    Device_BuckBoost buckboost_beta_;
    Device_BuckBoost buckboost_gamma_;

    Module_PowerCtrl_Param param_;

    Module_Status *status_;
    Module_SampleManager *sampler_;
} Module_PowerCtrl;

void Module_PowerCtrl_Init(Module_PowerCtrl *this, Module_PowerCtrl_Param param);
void Module_PowerCtrl_Control(Module_PowerCtrl *this);