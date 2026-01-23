#pragma once
#include "bsp_hrtim.h"
#include "comp_utils.h"

typedef struct {
  float CAP_IOUT_MAX;
  float CAP_CUTOFF_VOLTAGE;
  float CAP_IOUT_MIN;
  float CAP_NORMAL_VOLTAGE;
  float CAP_MAX_VOLTAGE;
  float I_LIMIT;
  float BAT_VOLTAGE_MIN;
} Device_BuckBoost_Param;

typedef struct {
  const float CAP_IOUT_MAX;
  const float CAP_IOUT_MIN;
  const float CAP_CUTOFF_VOLTAGE;
  const float CAP_NORMAL_VOLTAGE;
  const float CAP_MAX_VOLTAGE;
  const float I_LIMIT;
  const float BAT_VOLTAGE_MIN;
} Device_BuckBoost;

static inline void Device_BuckBoost_Init(Device_BuckBoost *this,
                                         Device_BuckBoost_Param param) {
  *(float *)&this->CAP_IOUT_MIN = param.CAP_IOUT_MIN;
  *(float *)&this->CAP_IOUT_MAX = param.CAP_IOUT_MAX;
  *(float *)&this->CAP_CUTOFF_VOLTAGE = param.CAP_CUTOFF_VOLTAGE;
  *(float *)&this->CAP_NORMAL_VOLTAGE = param.CAP_NORMAL_VOLTAGE;
  *(float *)&this->CAP_MAX_VOLTAGE = param.CAP_MAX_VOLTAGE;
  *(float *)&this->I_LIMIT = param.I_LIMIT;
  *(float *)&this->BAT_VOLTAGE_MIN = param.BAT_VOLTAGE_MIN;
}

static inline void __attribute__((always_inline)) Device_BuckBoost_Disable() {
  bsp_hrtim_muti_stop(BSP_HRTIM_A, BSP_HRTIM_B);
}

static inline void __attribute__((always_inline)) Device_BuckBoost_Enable() {
  // TODO:

  bsp_hrtim_muti_start(BSP_HRTIM_A, BSP_HRTIM_B);
}

static inline void __attribute__((always_inline))
Device_BuckBoost_UpdatePWM(float VBToVA) {

  static bool buckBoostMode = false;
  float dutyA = 0.0f;
  float dutyB = 0.0f;

  clampf(&VBToVA, 0.05f, 10.0f);

  if (buckBoostMode) {
    if (VBToVA < 0.8f || VBToVA > 1.25f)
      buckBoostMode = false;
  } else {
    if (VBToVA > 0.9f && VBToVA < 1.111f)
      buckBoostMode = true;
  }

  if (buckBoostMode) {
    dutyA = (VBToVA + 1.0f) * 0.4f;
    dutyB = (1.0f / VBToVA + 1.0f) * 0.4f;
  } else {
    if (VBToVA < 1) {
      dutyA = 0.9f * VBToVA;
      dutyB = 0.9f;
    } else {
      dutyA = 0.9f;
      dutyB = 0.9f / VBToVA;
    }
  }
  bsp_hrtim_set_comp(BSP_HRTIM_A, dutyA);
  bsp_hrtim_set_comp(BSP_HRTIM_B, dutyB);
}
