#define BSP_ADC_IMPLEMENTATION
#include "SuperCap.h"

#include <stdint.h>

#include "bsp_adc.h"
#include "bsp_time.h"
#include "dev_buckboost.h"
#include "dev_buzzer.h"
#include "dev_led.h"
#include "mod_errchecker.h"
#include "mod_powerctrl.h"
#include "mod_status.h"

static SuperCap supercap;

#define CPU_FREQ_HZ (170000000UL)
#define HRTIM_ISR_CYCLE_BUDGET ((uint32_t)(DT * (float)CPU_FREQ_HZ + 0.5f))

void SuperCap_Init(SuperCap *this, SuperCap_Param param)
{
    Module_Sampler_Init(&supercap.sampler_, param.sampler);
    // Module_ErrChecker_Init(&supercap.errchk_, param.errchk);
    Module_PowerCtrl_Init(&this->powerctrl_, param.powerctrl);
    bsp_time_hs_start();
    bsp_time_ls_start();
    Device_LED_Init();
    Device_BuckBoost_Enable();
    // 1. 开启 CoreDebug 中的 TRCENA 位，允许使用跟踪组件
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // 2. 将 DWT 计数器清零
    DWT->CYCCNT = 0;
    // 3. 开启 DWT 控制寄存器中的 CYCCNTENA 位，开始计数
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

#define DT (24000.0f * 8.0f / (170000000.0f * 32.0f))  // HRTIM MREP实际控制周期，约17.647us / 56.667kHz

void SuperCap_Start()
{
    /* clang-format off */
SuperCap_Param param_ = {
    .sampler = {
        .dt = DT, // HRTIM MREP actual loop rate: about 56.667kHz
        .vaside = {
            .adc_channel = BSP_ADC_VA,
            .k = 0.0073137736f,
            .b = (-0.0561247502f),
            .cutoff_freq = 150.0f
        },
        .vbside ={
            .adc_channel = BSP_ADC_VB,
            .k = 0.0072455170f,
            .b = (-0.0428535364f),
            .cutoff_freq = 150.0f
        },
        .iaside = {
            .adc_channel = BSP_ADC_IA,
            .k =   0.0156076632f,
            .b = (-31.8713917797f),
            .cutoff_freq = 150.0f
        },
        .i_alpha = {
            .adc_channel = BSP_ADC_Ialpha,
            .k = (-0.0171938062f),
            .b = 35.0536976581f,
            .cutoff_freq = 150.0f
        },
        .i_beta = {
            .adc_channel = BSP_ADC_Ibeta,
            .k = (-0.0173240675f),
            .b = 35.3941995088f,
            .cutoff_freq = 150.0f
        },
        .i_gamma = {
            .adc_channel = BSP_ADC_Igamma,
            .k = (-0.0169229675f),
            .b = 34.5661755549f,
            .cutoff_freq = 150.0f
        },
        .iRefree = {
            .adc_channel = BSP_ADC_IREF,
            .k =   0.0140179631f,
            .b = (-28.5919519601f),
            .cutoff_freq = 150.0f
        }

    },
    .powerctrl = {
        .dt = DT, // HRTIM MREP actual loop rate: about 56.667kHz
        .sampler_ = &(supercap.sampler_),
        .status_ = &(supercap.status_),
        .default_energy = 60.0f,
        .default_output_duty = 0.001f,
        .default_base_referee_power = 60.0f,
        .k_feedforward = 0.0f,
        .vbside = {
            .k = 0.01f,
            .p = 0.01f,
            .i = 0.01f,
            .i_limit = 10.0f,
            .out_limit = 30.0f
        },
        .ialpha = {
            .k = 1.3f,
            .p = 0.46f,
            .i = 0.3f,
            .i_limit = 10.0f,
            .out_limit = 10.0f
        },
        .ibeta = {
            .k = 1.3f,
            .p = 0.46f,
            .i = 0.3f,
            .i_limit = 10.0f,
            .out_limit = 10.0f
        },
        .igamma = {
            .k = 1.3f,
            .p = 0.46f,
            .i = 0.3f,
            .i_limit = 10.0f,
            .out_limit = 10.0f
        },
        .preferee = {
            .k = 1.1f,
            .p = 0.53f,
            .i = 1.1f,
            .i_limit = 100.0f,
            .out_limit = 130.0f
        },
        .energy = {
            .k = 0.01f,
            .p = 0.01f,
            .i = 10.0f,
            .i_limit = 10.0f,
            .out_limit = 30.0f
        },
        .buckboost = {
            .CAP_CUTOFF_VOLTAGE = 0.1f,
            .CAP_MAX_VOLTAGE = 28.8f,
            .CAP_NORMAL_VOLTAGE = 12.0f,
            .CAP_IOUT_MAX = 22.5f,
            .CAP_IOUT_MIN = 0.1f,
            .I_LIMIT = 22.5f,
            .BAT_VOLTAGE_MIN = 10.0f
        }
    },
    .errchk = {
        .SHORT_CIRCUIT_VOLTAGE = 4.0f,
        .SHORT_CIRCUIT_CURRENT = 20.0f,
        .sampler_ = &(supercap.sampler_),
        .status_ = &(supercap.status_),
    }
};
/*clang-format on*/

    SuperCap_Init(&supercap,param_);
    Device_Buzzer_PowerOn();
}

inline void __attribute__((always_inline))  SuperCap_control(){
  
    Module_Sampler_Update(&(supercap.sampler_));
    //Module_ErrChecker_ShortChk(&(supercap.errchk_));
    Module_PowerCtrl_Control(&(supercap.powerctrl_));

}

volatile bool blocking;


volatile uint32_t ALLt = 0;
volatile uint32_t supercap_irq_cycles_last = 0;
volatile uint32_t supercap_irq_cycles_max = 0;
volatile uint32_t supercap_irq_load_permille_last = 0;
volatile uint32_t supercap_irq_load_permille_max = 0;
/**
 * @brief 64khz control cycle ,pid\pwm update\short check
 *
 */
void HRTIM1_Master_IRQHandler(void) {

    uint32_t cycle_start = DWT->CYCCNT;

    __HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1, HRTIM_MASTER_IT_MREP);
    SuperCap_control();


  if (__HAL_HRTIM_MASTER_GET_FLAG(&hhrtim1, HRTIM_MASTER_FLAG_MREP) !=
      RESET) // blocking detected
  {
    blocking = true;
    ALLt++;
    Device_Buzzer_Play(1800 * 0.65f, 1.0f);
    __HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1,HRTIM_MASTER_IT_MREP); // stall the loop 
  }else{
    blocking = false;
  }

  uint32_t cycle_cost = DWT->CYCCNT - cycle_start;
  supercap_irq_cycles_last = cycle_cost;
  if (cycle_cost > supercap_irq_cycles_max) {
    supercap_irq_cycles_max = cycle_cost;
  }

  supercap_irq_load_permille_last =
      (uint32_t)(((uint64_t)cycle_cost * 1000ULL) / HRTIM_ISR_CYCLE_BUDGET);
  supercap_irq_load_permille_max =
      (uint32_t)(((uint64_t)supercap_irq_cycles_max * 1000ULL) / HRTIM_ISR_CYCLE_BUDGET);
}

/**
 * @brief 1khz control
 *
 */
void TIM2_IRQHandler(void) { 
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
 }
