#include "bsp_pwm.h"

#include "main.h"
#include "stm32f334x8.h"
#include "stm32f3xx_hal_hrtim.h"
#include "stm32f3xx_hal_tim.h"

extern TIM_HandleTypeDef hhrtim1; // master
extern TIM_HandleTypeDef htim1;   // buzzer

typedef struct {
  TIM_HandleTypeDef *tim;
  uint32_t channel;
} bsp_pwm_config_t;

static bsp_pwm_config_t bsp_pwm_map[BSP_PWM_NUM] = {
    [BSP_PWM_A] = {&hhrtim1, HRTIM_TIMERID_TIMER_A},
    [BSP_PWM_B] = {&hhrtim1, HRTIM_TIMERID_TIMER_B},
    [BSP_PWM_BUZZER] = {&htim1, TIM_CHANNEL_2}};

/**
 * @brief start pwm (TODO: Check channel mode - single or waveform)
 *
 * @param ch
 * @return bsp_status_t
 */
bsp_status_t bsp_pwm_start(bsp_pwm_channel_t ch) {

  htim1.Instance->CCR2 = 0U;
  HAL_TIMEx_PWMN_Start(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);

  return BSP_OK;
}

/**
 * @brief
 *
 * @param ch
 * @param duty_cycle
 * @return bsp_status_t
 */
bsp_status_t bsp_pwm_set_comp(bsp_pwm_channel_t ch, float duty_cycle) {
  if (duty_cycle > 1.0f) {
    return BSP_ERR;
  }

  if (duty_cycle < 0.0f) {
    duty_cycle = 0.f;
  }

  /* 通过PWM通道对应定时器重载值和给定占空比，计算PWM周期值 */
  uint16_t pulse = (uint16_t)(duty_cycle * (float)__HAL_TIM_GET_AUTORELOAD(
                                               bsp_pwm_map[ch].tim));

  __HAL_TIM_SET_COMPARE(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel, pulse);

  return BSP_OK;
}

bsp_status_t bsp_pwm_set_freq(bsp_pwm_channel_t ch, float freq) {
  uint16_t reload = (uint16_t)(50000U / freq);

  if (reload > 0) {
    __HAL_TIM_PRESCALER(bsp_pwm_map[ch].tim, reload);
  } else {
    return BSP_ERR;
  }

  return BSP_OK;
}

// // TODO : Adapt to current hardware bsp_status_t
// bsp_pwm_set_realfreq(bsp_pwm_channel_t ch, float freq) {
//   uint16_t arr = 0;
//   if (bsp_pwm_map[ch].tim == &htim3 || bsp_pwm_map[ch].tim == &htim4 ||
//       bsp_pwm_map[ch].tim == &htim5) {
//     arr = (uint16_t)(500000U / freq);
//   } else if (bsp_pwm_map[ch].tim == &htim1 || bsp_pwm_map[ch].tim == &htim8)
//   {
//     arr = (uint16_t)(1000000U / freq);
//   }

//   if (arr > 0) {
//     __HAL_TIM_SET_AUTORELOAD(bsp_pwm_map[ch].tim, arr);
//   } else {
//     return BSP_ERR;
//   }

// return BSP_OK;
// }

bsp_status_t bsp_pwm_stop(bsp_pwm_channel_t ch) {
  HAL_TIM_PWM_Stop(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  return BSP_OK;
}
