#pragma once

#include "bsp.h"

typedef enum { BSP_HRTIM_A, BSP_HRTIM_B, BSP_HRTIM_NUM } bsp_hrtim_channel_t;

bsp_status_t bsp_hrtim_start(bsp_hrtim_channel_t ch);
bsp_status_t bsp_hrtim_stop(bsp_hrtim_channel_t ch);
bsp_status_t bsp_hrtim_set_comp(bsp_hrtim_channel_t ch, float duty_cycle);
