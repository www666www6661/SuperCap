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

typedef enum
{
    BUCK,
    BOOST,
    BUCKBOOST,
} Device_BuckBoostMode_t;

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

static inline Device_BuckBoostMode_t __attribute__((always_inline)) Device_BuckBoost_GetMode(float VBToVA)
{
    static Device_BuckBoostMode_t mode = BUCK;

    if (mode == BUCKBOOST)
    {
        if (VBToVA < 0.90f || VBToVA > 1.10f)  // BUCKBOOST退出区稍宽，避免模式抖动
            mode = (VBToVA < 1.0f) ? BUCK : BOOST;
    }
    else
    {
        if (VBToVA > 0.97f && VBToVA < 1.03f)  // 只在接近1:1时进入BUCKBOOST
        {
            mode = BUCKBOOST;
        }
        else
        {
            mode = (VBToVA < 1.0f) ? BUCK : BOOST;
        }
    }
    return mode;
}

static inline void __attribute__((always_inline)) Device_BuckBoost_UpdatePWM(Device_BuckBoost *this, float VBToVA, Device_BuckBoostMode_t mode)
{
    float dutyA = 0.0f;
    float dutyB = 0.0f;

    const float duty_max = 0.95f;        // duty上限
    const float duty_base = 0.90f;       // BUCK/BOOST固定边占空比
    const float buckboost_gain = 0.35f;  // BUCKBOOST系数，避免x≈1时占空比过高

    if (mode == BUCKBOOST)
    {
        VBToVA = CLAMP(VBToVA, 0.73f, 1.37f);  // 保证BUCKBOOST两边都不易饱和
        dutyA = buckboost_gain * (VBToVA + 1.0f);
        dutyB = buckboost_gain * (1.0f / VBToVA + 1.0f);
    }
    else if (mode == BUCK)
    {
        VBToVA = CLAMP(VBToVA, 0.20f, duty_max / duty_base);  // BUCK线性区限幅
        dutyA = duty_base * VBToVA;
        dutyB = duty_base;
    }
    else if (mode == BOOST)
    {
        VBToVA = CLAMP(VBToVA, duty_base / duty_max, 3.0f);  // BOOST线性区限幅
        dutyA = duty_base;
        dutyB = duty_base / VBToVA;
    }

    dutyA = CLAMP(dutyA, 0.0f, duty_max);
    dutyB = CLAMP(dutyB, 0.0f, duty_max);

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
