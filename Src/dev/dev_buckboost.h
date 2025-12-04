#pragma once
#include "bsp_hrtim.h"
#include "comp_utils.h"
#include <stdbool.h>

static inline void Device_BuckBoost_Disable() {
  bsp_hrtim_muti_stop(BSP_HRTIM_A, BSP_HRTIM_B);
}

static inline void Device_BuckBoost_Enable() {
  bsp_hrtim_muti_start(BSP_HRTIM_A, BSP_HRTIM_B);
}

static inline void Device_BuckBoost_UpdatePWM(float VBToVA) {
  // Ported from updatePWM() in PowerManager.cpp
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
