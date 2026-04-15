#include "dev_buzzer.h"

#include "bsp_hrtim.h"
#include "bsp_pwm.h"
#include "stm32g4xx_hal.h"

void Device_Buzzer_Start() { bsp_pwm_start(BSP_PWM_BUZZER); }

void Device_Buzzer_Stop() { bsp_pwm_stop(BSP_PWM_BUZZER); }

void Device_Buzzer_Set(float freq, float duty_cycle)
{
    bsp_pwm_set_freq(BSP_PWM_BUZZER, freq);
    bsp_pwm_set_comp(BSP_PWM_BUZZER, duty_cycle);
}

void Device_Buzzer_Play(float freq, uint32_t duration_ms)
{
    Device_Buzzer_Set(freq, 0.5f);  // Default 50% duty cycle
    Device_Buzzer_Start();
    HAL_Delay(duration_ms);
    Device_Buzzer_Stop();
}

void Device_Buzzer_PowerOn()
{
    Device_Buzzer_Start();
    HAL_Delay(100);
    Device_Buzzer_Set(1046.50f * 0.65f, 0.5f);
    HAL_Delay(250);

    /* D5 */
    Device_Buzzer_Set(1174.66f * 0.65f, 0.5f);
    HAL_Delay(250);

    /* G5 */
    Device_Buzzer_Set(1567.98f * 0.65f, 0.5f);
    HAL_Delay(250);

    Device_Buzzer_Stop();
}
