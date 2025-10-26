#pragma once

#include "bsp.h"

#define ADC_COUNT 2         // adc count
#define ADC_CHANNEL_COUNT 3 // Channel count （采样通道数 per adc）
#define ADC_SAMPLE_COUNT 8  // Sample count （采样次数，保证精度）
#define ADC_BUFFER_SIZE (ADC_CHANNEL_COUNT * ADC_SAMPLE_COUNT)

typedef enum {
  BSP_ADC_IA,
  BSP_ADC_IREF,
  BSP_ADC_VA,
  BSP_ADC_IB,
  BSP_ADC_VB,
  BSP_ADC_NTC,
  BSP_ADC_NUM
} bsp_adc_channel_t;

// ADC Buffer
__attribute__((section(
    ".BUFFER"))) static uint32_t adcBuf[ADC_COUNT][ADC_BUFFER_SIZE] = {};

bsp_status_t inline bsp_adc_cal(bsp_adc_channel_t ch);
bsp_status_t inline bsp_adc_start(bsp_adc_channel_t ch);