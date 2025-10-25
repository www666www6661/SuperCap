#include "bsp_pwm.h"

#include "main.h"
#include "stm32f334x8.h"
#include "stm32f3xx_hal_cortex.h"
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

/**
 * @brief HRTIM 计数时钟频率 (Hz)
 * @note HRTIM_CLK = HRTIM_CLK_SRC * PrescalerRatio
 *       HRTIM_CLK_SRC = 144MHz (SystemCoreClock * 2)
 *       PrescalerRatio = 32
 *       HRTIM_CLK = 144,000,000 * 32 = 4,608,000,000 Hz
 */
#define HRTIM_CLK_HZ 4608000000.0f

extern HRTIM_HandleTypeDef hhrtim1; // master
extern TIM_HandleTypeDef htim1;     // buzzer

typedef struct {
  void *tim;
  uint32_t channel;
} bsp_pwm_config_t;

static bsp_pwm_config_t bsp_pwm_map[BSP_PWM_NUM] = {
    [BSP_PWM_A] = {&hhrtim1, HRTIM_TIMERID_TIMER_A},
    [BSP_PWM_B] = {&hhrtim1, HRTIM_TIMERID_TIMER_B},
    [BSP_PWM_BUZZER] = {&htim1, TIM_CHANNEL_2}};

/**
 * @brief Start PWM output on a specific channel.
 * @param ch The PWM channel to start.
 * @return bsp_status_t Status of the operation.
 * lock with broad
 */
bsp_status_t bsp_pwm_start(bsp_pwm_channel_t ch) {
  if (bsp_pwm_map[ch].tim == &htim1) {
    HAL_TIM_PWM_Start(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  } else if (bsp_pwm_map[ch].channel == HRTIM_TIMERID_TIMER_A) {

  } else if (bsp_pwm_map[ch].channel == HRTIM_TIMERID_TIMER_B) {
    HAL_HRTIM_WaveformCountStart(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
    HAL_HRTIM_WaveformOutputStart(bsp_pwm_map[ch].tim, HRTIM_OUTPUT_TB1);
    HAL_HRTIM_WaveformOutputStart(bsp_pwm_map[ch].tim, HRTIM_OUTPUT_TB2);
  }
  return BSP_OK;
}

/**
 * @brief Start complementary PWM output on a specific channel.
 * @param ch The PWM channel to start.
 * @return bsp_status_t Status of the operation.
 */
bsp_status_t inline bsp_pwmn_start(bsp_pwm_channel_t ch) {
  if (bsp_pwm_map[ch].tim == &htim1) {
    HAL_TIMEx_PWMN_Start(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  }
  // } else {
  //   // HRTIM does not have a direct equivalent for PWMN_Start
  //   // We can start the timer output instead
  //   HAL_HRTIM_WaveformOutputStart(bsp_pwm_map[ch].tim,
  //   bsp_pwm_map[ch].channel);
  // }
  return BSP_OK;
}

/**
 * @brief Set the duty cycle for a specific PWM channel.
 * @param ch The PWM channel to configure.
 * @param duty_cycle The duty cycle to set (from 0.0f to 1.0f).
 * @return bsp_status_t Status of the operation.
 */
bsp_status_t bsp_pwm_set_comp(bsp_pwm_channel_t ch, float duty_cycle) {
  if (duty_cycle > 1.0f) {
    return BSP_ERR;
  }

  if (duty_cycle < 0.0f) {
    duty_cycle = 0.f;
  }

  // if (bsp_pwm_map[ch].tim == &htim1) {
  TIM_HandleTypeDef *tim = (TIM_HandleTypeDef *)bsp_pwm_map[ch].tim;
  uint16_t pulse =
      (uint16_t)(duty_cycle * (float)__HAL_TIM_GET_AUTORELOAD(tim));
  __HAL_TIM_SET_COMPARE(tim, bsp_pwm_map[ch].channel, pulse);
  // } else {
  //   HRTIM_HandleTypeDef *hrtim = (HRTIM_HandleTypeDef
  //   *)bsp_pwm_map[ch].tim; uint32_t period = __HAL_HRTIM_GET_PERIOD(hrtim,
  //   bsp_pwm_map[ch].channel); uint16_t pulse = (uint16_t)(duty_cycle *
  //   (float)period);
  //   __HAL_HRTIM_SET_COMPARE(hrtim, bsp_pwm_map[ch].channel,
  //   HRTIM_COMPAREUNIT_1,
  //                           pulse);
  // }

  return BSP_OK;
}

/**
 * @brief Set the frequency for a specific PWM channel.
 * @param ch The PWM channel to configure.
 * @param freq The frequency to set in Hz.
 * @return bsp_status_t Status of the operation.
 */
bsp_status_t bsp_pwm_set_freq(bsp_pwm_channel_t ch, float freq) {
  // if (bsp_pwm_map[ch].tim == &htim1) {
  TIM_HandleTypeDef *tim = (TIM_HandleTypeDef *)bsp_pwm_map[ch].tim;
  uint32_t reload = (uint32_t)(BUZZER_TIM_CLK_HZ / freq);
  if (reload > 0) {
    __HAL_TIM_SET_AUTORELOAD(tim, reload - 1);
  } else {
    return BSP_ERR;
  }
  // } else {
  //   HRTIM_HandleTypeDef *hrtim = (HRTIM_HandleTypeDef
  //   *)bsp_pwm_map[ch].tim; uint32_t reload = (uint32_t)(HRTIM_CLK_HZ /
  //   freq); if (reload > 0) {
  //     __HAL_HRTIM_SET_PERIOD(
  //         hrtim, (TIM_HandleTypeDef *)bsp_pwm_map[ch].channel, reload - 1);
  //   } else {
  //     return BSP_ERR;
  //   }
  // }

  return BSP_OK;
}

bsp_status_t bsp_pwm_stop(bsp_pwm_channel_t ch) {
  if (bsp_pwm_map[ch].tim == &htim1) {
    HAL_TIM_PWM_Stop(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  } else {
    HAL_HRTIM_WaveformOutputStop(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  }
  return BSP_OK;
}

bsp_status_t bsp_pwmn_stop(bsp_pwm_channel_t ch) {
  if (bsp_pwm_map[ch].tim == &htim1) {
    HAL_TIMEx_PWMN_Stop(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  }

  return BSP_OK;
}

// 兼容妥协部分

bsp_status_t HRTIM_PWM_Start(uint32_t OutputChannel) {
  if (OutputChannel == HRTIM_TIMERID_TIMER_A)
    HAL_HRTIM_WaveformCountStart(bsp_pwm_map[ch].tim, bsp_pwm_map[ch].channel);
  HAL_HRTIM_WaveformOutputStart(bsp_pwm_map[ch].tim, HRTIM_OUTPUT_TA1);
  HAL_HRTIM_WaveformOutputStart(bsp_pwm_map[ch].tim, HRTIM_OUTPUT_TA2);
}
