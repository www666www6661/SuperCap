#include "SuperCap.h"
#include "bsp_time.h"
#include "dev_buckboost.h"
#include "dev_buzzer.h"
#include "mod_powerctrl.h"
#include "mod_status.h"

static SuperCap supercap;

void SuperCap_Init(SuperCap *this, SuperCap_Param param) {
  Module_Sampler_Init(&supercap.sampler_, param.sampler);
  Module_PowerCtrl_Init(&this->powerctrl_, param.powerctrl);
  bsp_time_hs_start();
  bsp_time_ls_start();
  Device_BuckBoost_Enable();
}

#define DT 1.0f / 64000.0f

void SuperCap_Start() {
  /* clang-format off */
SuperCap_Param param_ = {
    .sampler = {
        .dt = DT, //64khz runing
        .vaside = {
            .adc_channel = BSP_ADC_VA,
            .k = 0.00807537f,
            .b = 0.02422611f,
            .cutoff_freq = 500.0f
        },
        .vbside ={
            .adc_channel = BSP_ADC_VB,
            .k = 0.008096386f,
            .b = 0.041638554f,
            .cutoff_freq = 500.0f
        },
        .iaside = {
            .adc_channel = BSP_ADC_IA,
            .k = 0.020190366f,
            .b = (-41.28785694f),
            .cutoff_freq = 3000.0f
        },
        .ibside = {
            .adc_channel = BSP_ADC_IB,
            .k = (-0.020219526f),
            .b = 41.32986713f,
            .cutoff_freq = 3000.0f
        },
        .iRefree = {
            .adc_channel = BSP_ADC_IREF,
            .k = 0.020207852f,
            .b = (-41.30051963f),
            .cutoff_freq = 3000.0f
        }

    },
    .powerctrl = {
        .dt = DT, //64khz runing
        .sampler_ = &(supercap.sampler_),
        .status_ = &(supercap.status_),
        .default_energy = 60.0f,
        .default_output_duty = 0.001f,
        .default_base_referee_power = 60.0f,
        .vbside = {
            .k = 0.00001f,
            .p = 0.00001f,
            .i = 0.0f,
            .d = 0.0f,
            .i_limit = 0.0f,
            .out_limit = 0.0f,
            .d_cutoff_freq = 0.0f
        },
        .iaside = {
            .k = 0.00001f,
            .p = 0.00001f,
            .i = 0.0f,
            .d = 0.0f,
            .i_limit = 0.0f,
            .out_limit = 0.0f,
            .d_cutoff_freq = 0.0f
        },
        .preferee = {
            .k = 0.0001f,
            .p = 0.0001f,
            .i = 0.0f,
            .d = 0.0f,
            .i_limit = 0.0f,
            .out_limit = 0.0f,
            .d_cutoff_freq = 0.0f
        },
        .energy = {
            .k = 0.0001f,
            .p = 0.0001f,
            .i = 0.0f,
            .d = 0.0f,
            .i_limit = 0.0f,
            .out_limit = 0.0f,
            .d_cutoff_freq = 0.0f
        },
        .buckboost = {
            .CAP_CUTOFF_VOLTAGE = 5.0f,
            .CAP_MAX_VOLTAGE = 28.8f,
            .CAP_NORMAL_VOLTAGE = 12.0f,
            .CAP_IOUT_MAX = 22.5f,
            .CAP_IOUT_MIN = 0.1f,
            .I_LIMIT = 22.5f
        }

    }
};
/*clang-format on*/

    SuperCap_Init(&supercap,param_);
    Device_Buzzer_PowerOn();
}
inline void __attribute__((always_inline))  SuperCap_control(){
    Module_Sampler_Update(&(supercap.sampler_));
    Module_PowerCtrl_Calculate(&(supercap.powerctrl_));
}


/**
 * @brief 64khz control cycle ,pid\pwm update\short check
 *
 */
void HRTIM1_Master_IRQHandler(void) {

  __HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1, HRTIM_MASTER_IT_MREP);
    SuperCap_control();


  if (__HAL_HRTIM_MASTER_GET_FLAG(&hhrtim1, HRTIM_MASTER_FLAG_MREP) !=
      RESET) // blocking detected
  {
    // TODO:blocking = true;
    __HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1,
                                HRTIM_MASTER_IT_MREP); // stall the loop
  }
}

/**
 * @brief 1khz control
 *
 */
void TIM2_IRQHandler(void) { 
    
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
 }
