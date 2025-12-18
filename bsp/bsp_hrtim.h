#pragma once

#include "bsp.h"
#include "hrtim.h"
#include <stdbool.h>

#define HRTIM_PERIOD 16000.0f // defined on stm32cubemx

typedef enum { BSP_HRTIM_A, BSP_HRTIM_B, BSP_HRTIM_NUM } bsp_hrtim_channel_t;

typedef struct {
  uint32_t timer_id;
  uint32_t timer_idx;
  uint32_t output_1;
  uint32_t output_2;
} bsp_hrtim_config_t;

extern HRTIM_HandleTypeDef hhrtim1;
static const bsp_hrtim_config_t bsp_hrtim_map[BSP_HRTIM_NUM] = {
    [BSP_HRTIM_A] = {.timer_id = HRTIM_TIMERID_TIMER_A,
                     .timer_idx = HRTIM_TIMERINDEX_TIMER_A,
                     .output_1 = HRTIM_OUTPUT_TA1,
                     .output_2 = HRTIM_OUTPUT_TA2},
    [BSP_HRTIM_B] = {.timer_id = HRTIM_TIMERID_TIMER_B,
                     .timer_idx = HRTIM_TIMERINDEX_TIMER_B,
                     .output_1 = HRTIM_OUTPUT_TB1,
                     .output_2 = HRTIM_OUTPUT_TB2}};

/**
 * @brief Start HRTIM output on a specific channel.
 * @param ch The HRTIM channel to start.
 * @return bsp_status_t Status of the operation.
 */
static inline bsp_status_t bsp_hrtim_start(bsp_hrtim_channel_t ch) {
  HAL_HRTIM_WaveformCountStart(&hhrtim1, bsp_hrtim_map[ch].timer_id);
  HAL_Delay(1); // 等待波形对齐（可能有效吧）
  HAL_HRTIM_WaveformOutputStart(&hhrtim1, bsp_hrtim_map[ch].output_1 |
                                              bsp_hrtim_map[ch].output_2);
  return BSP_OK;
}

/**
 * @brief Start HRTIM output on muti channel.
 * @param ch The HRTIM channel to start.
 * @return bsp_status_t Status of the operation.
 */
static inline bsp_status_t bsp_hrtim_muti_start(bsp_hrtim_channel_t ch1,
                                                bsp_hrtim_channel_t ch2) {
  HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_MASTER |
                                             bsp_hrtim_map[ch1].timer_id |
                                             bsp_hrtim_map[ch2].timer_id);
  HAL_Delay(1); // 等待波形对齐（可能有效吧）
  HAL_HRTIM_WaveformOutputStart(
      &hhrtim1, bsp_hrtim_map[ch1].output_1 | bsp_hrtim_map[ch1].output_2 |
                    bsp_hrtim_map[ch2].output_1 | bsp_hrtim_map[ch2].output_2);
  return BSP_OK;
}

/**
 * @brief Stop HRTIM output on a specific channel.
 * @param ch The HRTIM channel to stop.
 * @return bsp_status_t Status of the operation.
 */
static inline bsp_status_t bsp_hrtim_stop(bsp_hrtim_channel_t ch) {
  HAL_HRTIM_WaveformOutputStop(&hhrtim1, bsp_hrtim_map[ch].output_1 |
                                             bsp_hrtim_map[ch].output_2);
  return BSP_OK;
}

/**
 * @brief Stop HRTIM output on muti channel
 *
 * @param ch1
 * @param ch2
 * @return bsp_status_t
 */
static inline bsp_status_t bsp_hrtim_muti_stop(bsp_hrtim_channel_t ch1,
                                               bsp_hrtim_channel_t ch2) {
  HAL_HRTIM_WaveformOutputStop(
      &hhrtim1, bsp_hrtim_map[ch1].output_1 | bsp_hrtim_map[ch1].output_2 |
                    bsp_hrtim_map[ch2].output_1 | bsp_hrtim_map[ch2].output_2);
  return BSP_OK;
}

/**
 * @brief Set the duty cycle for a specific HRTIM channel.
 * @param ch The HRTIM channel to configure.
 * @param duty_cycle The duty cycle to set (from 0.0000～01f to 0.9999～f).
 * @return bsp_status_t Status of the operation.
 */
static inline bsp_status_t bsp_hrtim_set_comp(bsp_hrtim_channel_t ch,
                                              float duty_cycle) {
  static HRTIM_CompareCfgTypeDef compare_config = {0};

  compare_config.CompareValue = HRTIM_PERIOD / 2 * (1 - duty_cycle);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, bsp_hrtim_map[ch].timer_idx,
                                  HRTIM_COMPAREUNIT_1, &compare_config);
  compare_config.CompareValue = HRTIM_PERIOD / 2 * (1 + duty_cycle);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, bsp_hrtim_map[ch].timer_idx,
                                  HRTIM_COMPAREUNIT_3, &compare_config);
  return BSP_OK;
}