#pragma once
#include "dev_sampler.h"

typedef struct Sampler_Param {
  Device_Sampler_Param vaside;
  Device_Sampler_Param vbside;
  Device_Sampler_Param iaside;
  Device_Sampler_Param ibside;
  Device_Sampler_Param iRefree;
  Device_Sampler_Param iChassis;
} Module_SampleManager_Param;

typedef struct Sampler {
  Device_Volt_Sampler vaside_;
  Device_Volt_Sampler vbside_;
  Device_Current_Sampler iaside_;
  Device_Current_Sampler ibside_;
  Device_Current_Sampler iRefree_;
  Device_Current_Sampler iChassis_;
  Module_SampleManager_Param param_;
} Module_SampleManager;

static inline void Module_Sampler_Init(Module_SampleManager *this,
                                       Module_SampleManager_Param param) {
  this->param_ = param;
  Device_Volt_Sampler_Init(&this->vaside_, this->param_.vaside);
  Device_Volt_Sampler_Init(&this->vbside_, this->param_.vbside);
  Device_Current_Sampler_Init(&this->iaside_, this->param_.iaside);
  Device_Current_Sampler_Init(&this->ibside_, this->param_.ibside);
  Device_Current_Sampler_Init(&this->iRefree_, this->param_.iRefree);
  Device_Current_Sampler_Init(&this->iChassis_, this->param_.iChassis);
}
