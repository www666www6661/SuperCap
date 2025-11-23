#include "bsp_hrtim.h"
#include "hrtim.h"
#include "stm32f3xx_hal_hrtim.h"

#define HRTIM_PERIOD 16000u // defined on stm32cubemx

extern HRTIM_HandleTypeDef hhrtim1;

typedef struct {
  uint32_t timer_id;
  uint32_t timer_idx;
  uint32_t output_1;
  uint32_t output_2;
} bsp_hrtim_config_t;

static bsp_hrtim_config_t bsp_hrtim_map[BSP_HRTIM_NUM] = {
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
inline bsp_status_t bsp_hrtim_start(bsp_hrtim_channel_t ch) {
  HAL_HRTIM_WaveformCountStart(&hhrtim1, bsp_hrtim_map[ch].timer_id);
  HAL_HRTIM_WaveformOutputStart(&hhrtim1, bsp_hrtim_map[ch].output_1 |
                                              bsp_hrtim_map[ch].output_2);
  return BSP_OK;
}

/**
 * @brief Stop HRTIM output on a specific channel.
 * @param ch The HRTIM channel to stop.
 * @return bsp_status_t Status of the operation.
 */
inline bsp_status_t bsp_hrtim_stop(bsp_hrtim_channel_t ch) {
  HAL_HRTIM_WaveformOutputStop(&hhrtim1, bsp_hrtim_map[ch].output_1 |
                                             bsp_hrtim_map[ch].output_2);
  return BSP_OK;
}

/**
 * @brief Set the duty cycle for a specific HRTIM channel.
 * @param ch The HRTIM channel to configure.
 * @param duty_cycle The duty cycle to set (from 0.0f to 1.0f).
 * @return bsp_status_t Status of the operation.
 */
bsp_status_t bsp_hrtim_set_comp(bsp_hrtim_channel_t ch, float duty_cycle) {
  HRTIM_CompareCfgTypeDef compare_config = {0};

  compare_config.CompareValue = HRTIM_PERIOD / 2 * (1 - duty_cycle);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, bsp_hrtim_map[ch].timer_idx,
                                  HRTIM_COMPAREUNIT_1, &compare_config);
  compare_config.CompareValue = HRTIM_PERIOD / 2 * (1 + duty_cycle);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, bsp_hrtim_map[ch].timer_idx,
                                  HRTIM_COMPAREUNIT_3, &compare_config);
  return BSP_OK;
}
