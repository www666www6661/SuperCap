#include "dev_buzzer.h"

#include "bsp_pwm.h"
#include "stm32f3xx_hal.h"
#include "tim.h"

bool Buzzer_Start() { return bsp_pwm_start(BSP_PWM_BUZZER) == BSP_OK; }

bool Buzzer_Stop() { return bsp_pwm_stop(BSP_PWM_BUZZER) == BSP_OK; }

bool Buzzer_Set(float freq, float duty_cycle) {
  bsp_pwm_set_comp(BSP_PWM_BUZZER, duty_cycle);
  return bsp_pwm_set_freq(BSP_PWM_BUZZER, freq) == BSP_OK;
}

void Buzzer_PowerOn() {
  uint32_t arr = (uint32_t)(50000U / 1300) - 1U;
  __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, arr / 3U);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  // Buzzer_Stop();
}
