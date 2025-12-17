#include <stdint.h>

#include "adc.h"
#include "bsp.h"
#include "bsp_adc.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_adc.h"

#define ADC_COUNT 2         // adc count
#define ADC_CHANNEL_COUNT 3 // Channel count （采样通道数 per adc）
#define ADC_SAMPLE_COUNT 8  // Sample count （采样次数，保证精度）
#define ADC_BUFFER_SIZE (ADC_CHANNEL_COUNT * ADC_SAMPLE_COUNT)
#define ADC_DEV_(arg) &hadc##arg, (arg - 1) // adc设备映射

// ADC Buffer
__attribute__((section(
    ".BUFFER"))) static uint32_t adcBuf[ADC_COUNT][ADC_BUFFER_SIZE] = {0};
typedef struct {
  void *adc;
  uint32_t dev;
  uint32_t offset;
} bsp_adc_config_t;

bsp_adc_config_t bsp_adc_map[BSP_ADC_NUM] = {
    [BSP_ADC_VA] = {ADC_DEV_(1), 3 - 1},
    [BSP_ADC_IA] = {ADC_DEV_(1), 1 - 1},
    [BSP_ADC_IREF] = {ADC_DEV_(1), 2 - 1},
    [BSP_ADC_VB] = {ADC_DEV_(2), 2 - 1},
    [BSP_ADC_IB] = {ADC_DEV_(2), 1 - 1},
    [BSP_ADC_NTC] = {ADC_DEV_(2), 3 - 1}};

uint16_t bsp_adc_sumBuf[BSP_ADC_NUM] = {0}; // SUM Buffer of adcValue

/**
 * @brief ADC Calibration
 *        only need be down twice here (adc1/2)
 * @param ch
 * @return bsp_status_t
 */
bsp_status_t bsp_adc_cal(bsp_adc_channel_t ch) {

  static uint8_t adc_calibrated[ADC_COUNT] = {0};

  if (adc_calibrated[bsp_adc_map[ch].dev] == 0) {
    adc_calibrated[bsp_adc_map[ch].dev] = 1;
    HAL_ADCEx_Calibration_Start(bsp_adc_map[ch].adc, ADC_SINGLE_ENDED);
  } else {
    return BSP_ERR; // Repeatedly calibration error or Channel error
  }

  return BSP_OK;
}

/**
 * @brief ADC start on DMA mode
 *        only need be down twice here (adc1/2)
 * @param ch
 * @return bsp_status_t
 */
bsp_status_t bsp_adc_start(bsp_adc_channel_t ch) {

  static uint32_t adc_running[ADC_COUNT] = {0};

  if (adc_running[bsp_adc_map[ch].dev] == 0) {
    adc_running[bsp_adc_map[ch].dev] = 1;
    bsp_adc_cal(ch);
    HAL_Delay(1000);
    HAL_ADC_Start_DMA(bsp_adc_map[ch].adc, adcBuf[bsp_adc_map[ch].dev],
                      ADC_BUFFER_SIZE);
  } else {
    return BSP_ERR; // Repeatedly start error or Channel error
  }

  return BSP_OK;
}

/**
 * @brief Update ADC sum buffer with accumulated sample values
 *
 * @param ch ADC channel to process
 * @return bsp_status_t BSP_OK on success, BSP_ERR on channel error
 *         the result will be stored in bsp_adc_sumBuf[ch]
 */
bsp_status_t bsp_adc_updatesumbuf(bsp_adc_channel_t ch) {

  if (ch >= BSP_ADC_NUM) {
    return BSP_ERR; // Channel error
  }

  bsp_adc_sumBuf[ch] = 0; // Clear sum buffer before accumulation

  for (uint32_t i = bsp_adc_map[ch].offset; i < ADC_BUFFER_SIZE;
       i += ADC_CHANNEL_COUNT) {
    bsp_adc_sumBuf[ch] += adcBuf[bsp_adc_map[ch].dev][i];
  }

  return BSP_OK;
}

/**
 * @brief dump buffer data into a specific buffer
 *
 * @param ch adc_channel
 * @param buf a pointer to target buffer
 */
inline void bsp_adc_dumpdata(bsp_adc_channel_t ch, uint16_t *buf) {
  *buf = bsp_adc_sumBuf[ch] / ADC_SAMPLE_COUNT;
}
