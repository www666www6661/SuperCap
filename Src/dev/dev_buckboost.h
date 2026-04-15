#pragma once
#include <stdbool.h>

#include "bsp_hrtim.h"
#include "comp_utils.h"

typedef enum
{
    PHAS_ALPHA,
    PHAS_BETA,
    PHAS_GAMMA,
    PHAS_NUM
} Device_BuckBoost_Phase_t;

typedef struct
{
    float CAP_IOUT_MAX;
    float CAP_CUTOFF_VOLTAGE;
    float CAP_IOUT_MIN;
    float CAP_NORMAL_VOLTAGE;
    float CAP_MAX_VOLTAGE;
    float I_LIMIT;
    float BAT_VOLTAGE_MIN;
    Device_BuckBoost_Phase_t phase;
} Device_BuckBoost_Param;

typedef struct
{
    const float CAP_IOUT_MAX;
    const float CAP_IOUT_MIN;
    const float CAP_CUTOFF_VOLTAGE;
    const float CAP_NORMAL_VOLTAGE;
    const float CAP_MAX_VOLTAGE;
    const float I_LIMIT;
    const float BAT_VOLTAGE_MIN;
    Device_BuckBoost_Phase_t phase;
} Device_BuckBoost;

static inline void Device_BuckBoost_Init(Device_BuckBoost *this, Device_BuckBoost_Param param)
{
    *(float *)&this->CAP_IOUT_MIN = param.CAP_IOUT_MIN;
    *(float *)&this->CAP_IOUT_MAX = param.CAP_IOUT_MAX;
    *(float *)&this->CAP_CUTOFF_VOLTAGE = param.CAP_CUTOFF_VOLTAGE;
    *(float *)&this->CAP_NORMAL_VOLTAGE = param.CAP_NORMAL_VOLTAGE;
    *(float *)&this->CAP_MAX_VOLTAGE = param.CAP_MAX_VOLTAGE;
    *(float *)&this->I_LIMIT = param.I_LIMIT;
    *(float *)&this->BAT_VOLTAGE_MIN = param.BAT_VOLTAGE_MIN;
    this->phase = param.phase;
}

static inline void __attribute__((always_inline)) Device_BuckBoost_Disable() { bsp_hrtim_allch_stop(); }

static inline void __attribute__((always_inline)) Device_BuckBoost_Enable() { bsp_hrtim_allch_start(); }

static inline bool __attribute__((always_inline)) Device_BuckBoost_GetBuckBoostMode(float Physical_VBToVA)
{
    static bool buckBoostMode = false;

    if (buckBoostMode)
    {
        if (Physical_VBToVA < 0.8f || Physical_VBToVA > 1.25f)
            buckBoostMode = false;
    }
    else
    {
        if (Physical_VBToVA > 0.9f && Physical_VBToVA < 1.111f)
            buckBoostMode = true;
    }
    return buckBoostMode;
}

static inline void __attribute__((always_inline)) Device_BuckBoost_UpdatePWM(Device_BuckBoost *this, float VBToVA, bool BuckBoostMode)
{
    float dutyA = 0.0f;
    float dutyB = 0.0f;

    if (BuckBoostMode)
    {
        dutyA = (VBToVA + 1.0f) * 0.4f;
        dutyB = (1.0f / VBToVA + 1.0f) * 0.4f;
    }
    else
    {
        if (VBToVA < 1.0f)
        {
            dutyA = 0.9f * VBToVA;
            dutyB = 0.9f;
        }
        else
        {
            dutyA = 0.9f;
            dutyB = 0.9f / VBToVA;
        }
    }

    switch (this->phase)
    {
    case PHAS_ALPHA:
        bsp_hrtim_set_comp(BSP_HRTIM_Aalpha, dutyA);
        bsp_hrtim_set_comp(BSP_HRTIM_Balpha, dutyB);
        break;
    case PHAS_BETA:
        bsp_hrtim_set_comp(BSP_HRTIM_Abeta, dutyA);
        bsp_hrtim_set_comp(BSP_HRTIM_Bbeta, dutyB);
        break;
    case PHAS_GAMMA:
        bsp_hrtim_set_comp(BSP_HRTIM_Agamma, dutyA);
        bsp_hrtim_set_comp(BSP_HRTIM_Bgamma, dutyB);
        break;
    default:
        break;
    }
}
