#include "dev_buzzer.h"

#include "bsp_hrtim.h"
#include "bsp_pwm.h"
#include "stm32f3xx_hal.h"

extern CAN_HandleTypeDef hcan;

void Buzzer_Start() { bsp_pwm_start(BSP_PWM_BUZZER); }

void Buzzer_Stop() { bsp_pwm_stop(BSP_PWM_BUZZER); }

void Buzzer_Set(float freq, float duty_cycle) {
  bsp_pwm_set_freq(BSP_PWM_BUZZER, freq);
  bsp_pwm_set_comp(BSP_PWM_BUZZER, duty_cycle);
}

void Buzzer_PowerOn() {
  bsp_hrtim_start(BSP_HRTIM_A);
  bsp_hrtim_set_comp(BSP_HRTIM_A, 0.99f);
  bsp_hrtim_start(BSP_HRTIM_B);
  bsp_hrtim_set_comp(BSP_HRTIM_B, 0.89f);

  Buzzer_Start();
  HAL_Delay(100);
  Buzzer_Set(1046.50f * 0.65f, 0.66f);
  HAL_Delay(250);

  /* D5 */
  Buzzer_Set(1174.66f * 0.65f, 0.66f);
  HAL_Delay(250);

  /* G5 */
  Buzzer_Set(1567.98f * 0.65f, 0.66f);
  HAL_Delay(250);

  Buzzer_Stop();
}
