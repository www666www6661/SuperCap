#pragma once

#include "bsp.h"

typedef enum {
  BSP_ADC_IA,
  BSP_ADC_IREF,
  BSP_ADC_VA,
  BSP_ADC_IB,
  BSP_ADC_VB,
  BSP_ADC_NTC,
  BSP_ADC_NUM
} bsp_adc_channel_t;

bsp_status_t bsp_adc_start(bsp_adc_channel_t ch);
bsp_status_t bsp_adc_updatesumbuf(bsp_adc_channel_t ch);
void bsp_adc_dumpdata(bsp_adc_channel_t ch, uint16_t *buf);