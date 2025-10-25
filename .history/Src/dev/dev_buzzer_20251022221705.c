#include "dev_buzzer.h"

#include "bsp_pwm.h"

bool Buzzer_Start() { return bsp_pwm_start(BSP_PWM_BUZZER) == BSP_OK; }

bool Buzzer_Stop() { return bsp_pwm_stop(BSP_PWM_BUZZER) == BSP_OK; }

bool Buzzer_Set(float freq, float duty_cycle) {
  bsp_pwm_set_comp(BSP_PWM_BUZZER, duty_cycle);
  return bsp_pwm_set_freq(BSP_PWM_BUZZER, freq) == BSP_OK;
}

void Buzzer_PowerOn() {
  /* C5 */
  Buzzer_Set(1046.50f, 0.99f);
  Start();
  HAL_Delay(250);

  /* D5 */
  Buzzer_Set(1174.66f, 0.99f);
  HAL_Delay(250);

  /* G5 */
  Buzzer_Set(1567.98f, 0.99f);
  HAL_Delay(250);

  /*P*/
  Buzzer_Set(0, 0);
  HAL_Delay(62);

  /*D5*/
  Buzzer_Set(1174.66f, 0.99f);
  HAL_Delay(200);

  /* G5 */
  Buzzer_Set(1567.98f, 0.99f);
  HAL_Delay(200);

  /*P*/
  Buzzer_Set(0, 0);
  HAL_Delay(62);

  /*D5*/
  Buzzer_Set(1147.66f, 0.99f);
  HAL_Delay(200);

  /* G5 */
  Buzzer_Set(1567.98f, 0.99f);
  HAL_Delay(200);

  Buzzer_Stop();
}