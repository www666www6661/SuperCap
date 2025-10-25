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
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  htim1.Instance->ARR = (uint32_t)(50000U / 1300) - 1U;
  htim1.Instance->CCR2 = htim1.Instance->ARR;
  htim1.Instance->CNT = 0U;
  // Buzzer_Stop();
}