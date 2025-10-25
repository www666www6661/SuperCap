#include "bsp_pwm.h"

#include "main.h"
#include "stm32f3xx_hal_hrtim.h"
#include "stm32f3xx_hal_tim.h"

/**
 * @brief TIM1 计数时钟频率 (Hz)
 * @note TIM_CLK = PCLK2 / (Prescaler + 1)
 *       PCLK2 = 72MHz (SystemCoreClock)
 *       Prescaler = 1439
 *       TIM_CLK = 72,000,000 / (1439 + 1) = 50,000 Hz
 */
#define BUZZER_TIM_CLK_HZ 50000.0f

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

bsp_status_t inline bsp_pwm_start(bsp_pwm_channel_t ch) {

  HAL_TIM_PWM_Start(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);

  return BSP_OK;
}

bsp_status_t inline bsp_pwmn_start(bsp_pwm_channel_t ch) {

  HAL_TIMEx_PWMN_Start(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);

  return BSP_OK;
}

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
  uint32_t reload = (uint32_t)(BUZZER_TIM_CLK_HZ / freq);

  if (reload > 0) {
    __HAL_TIM_SET_AUTORELOAD(bsp_pwm_map[ch].tim, reload - 1);
  } else {
    return BSP_ERR;
  }

  return BSP_OK;
}

bsp_status_t bsp_pwm_stop(bsp_pwm_channel_t ch) {
  HAL_TIM_PWM_Stop(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  return BSP_OK;
}

bsp_status_t bsp_pwmn_stop(bsp_pwm_channel_t ch) {
  HAL_TIMEx_PWMN_Stop(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  return BSP_OK;
}
