#include "dev_buzzer.h"

#include "bsp_pwm.h"
#include "stm32f3xx_hal.h"
#include "tim.h"

bool Buzzer_Start() { return bsp_pwm_start(BSP_PWM_BUZZER) == BSP_OK; }

bool Buzzer_Stop() { return bsp_pwm_stop(BSP_PWM_BUZZER) == BSP_OK; }

bool Buzzer_Set(float freq, float duty_cycle) {
  bsp_pwm_set_freq(BSP_PWM_BUZZER, freq);
  return bsp_pwm_set_comp(BSP_PWM_BUZZER, duty_cycle) == BSP_OK;
}

void Buzzer_PowerOn() {
  Buzzer_Set(1300.0f, 1.0f / 3.0f);
  Buzzer_Start();
  HAL_Delay(200) Buzzer_Stop();
}
