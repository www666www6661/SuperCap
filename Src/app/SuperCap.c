#include "SuperCap.h"
#include "bsp_adc.h"

SuperCap supercap;

void SuperCap_Init(SuperCap *this, SuperCap_Param param) {
  Module_Sampler_Init(&supercap.sampler_, param.sampler);
}

void SuperCap_Start() {
  /* clang-format off */
SuperCap_Param param_ ={
    .sampler ={
        .vaside = {
            .adc_channel = BSP_ADC_VA,
            .k = 0.00807537f,
            .b = 0.02422611f
        },
        .vbside ={
            .adc_channel = BSP_ADC_VB,
            .k = 0.008096386f,
            .b = 0.041638554f
        },
        .iaside = {
            .adc_channel = BSP_ADC_IA,
            .k = 0.020190366f,
            .b = (-41.28785694f)
        },
        .ibside = {
            .adc_channel = BSP_ADC_IB,
            .k = (-0.020219526f),
            .b = 41.32986713f
        },
        .iRefree = {
            .adc_channel = BSP_ADC_IREF,
            .k = 0.020207852f,
            .b = (-41.30051963f)
        }

    }
};
/*clang-format on*/

    SuperCap_Init(&supercap,param_); 
}

void SuperCap_control(){
    
}