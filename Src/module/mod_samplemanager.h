#pragma once

#include "comp_utils.h"
#include "dev_sampler.h"

typedef struct
{
    float dt;

    Device_Sampler_Param vaside;
    Device_Sampler_Param vbside;

    Device_Sampler_Param iaside;
    Device_Sampler_Param i_alpha;
    Device_Sampler_Param i_beta;
    Device_Sampler_Param i_gamma;

    Device_Sampler_Param iRefree;
} Module_SampleManager_Param;

typedef struct
{
    float dt;

    Device_Volt_Sampler vaside_;
    Device_Volt_Sampler vbside_;

    Device_Current_Sampler iaside_;
    Device_Current_Sampler i_alpha_;
    Device_Current_Sampler i_beta_;
    Device_Current_Sampler i_gamma_;

    Device_Current_Sampler iRefree_;

    Module_SampleManager_Param param_;
} Module_SampleManager;

static inline void Module_Sampler_Init(Module_SampleManager *this, Module_SampleManager_Param param)
{
    this->param_ = param;

    this->dt = this->param_.dt;

    Device_Volt_Sampler_Init(&this->vaside_, this->param_.vaside);
    Device_Volt_Sampler_Init(&this->vbside_, this->param_.vbside);

    Device_Current_Sampler_Init(&this->iaside_, this->param_.iaside);
    Device_Current_Sampler_Init(&this->i_alpha_, this->param_.i_alpha);
    Device_Current_Sampler_Init(&this->i_beta_, this->param_.i_beta);
    Device_Current_Sampler_Init(&this->i_gamma_, this->param_.i_gamma);

    Device_Current_Sampler_Init(&this->iRefree_, this->param_.iRefree);
}

/**
 * @brief This function can only read correct data when the ADC sampling cycle
 * is started.
 *
 * @param this
 */
static inline void __attribute__((always_inline)) Module_Sampler_Update(Module_SampleManager *this)
{
    Device_Sampler_GetVoltage(&(this->vaside_), this->dt);
    Device_Sampler_GetVoltage(&(this->vbside_), this->dt);

    Device_Sampler_GetCurrrent(&(this->iaside_), this->dt);
    Device_Sampler_GetCurrrent(&(this->i_alpha_), this->dt);
    Device_Sampler_GetCurrrent(&(this->i_beta_), this->dt);
    Device_Sampler_GetCurrrent(&(this->i_gamma_), this->dt);

    Device_Sampler_GetCurrrent(&(this->iRefree_), this->dt);
}
