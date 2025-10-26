#include <stdint.h>

#include "adc.h"
#include "bsp.h"
#include "bsp_adc.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_adc.h"

typedef struct {
  void *adc;
  uint32_t offset;
} bsp_adc_config_t;

bsp_adc_config_t bsp_adc_map[BSP_ADC_NUM] = {
    [BSP_ADC_VA] = {&hadc1, 1 - 1},   [BSP_ADC_IA] = {&hadc1, 2 - 1},
    [BSP_ADC_IREF] = {&hadc1, 3 - 1}, [BSP_ADC_VB] = {&hadc2, 1 - 1},
    [BSP_ADC_IB] = {&hadc2, 2 - 1},   [BSP_ADC_NTC] = {&hadc2, 3 - 1}};

uint16_t bsp_adc_sumBuf[BSP_ADC_NUM] = {}; // SUM Buffer of adcValue

/**
 * @brief ADC Calibration
 *        only need be down twice here (adc1/2)
 * @param ch
 * @return bsp_status_t
 */
bsp_status_t bsp_adc_cal(bsp_adc_channel_t ch) {

  static adc_calibrated[2] = {0};

  if (bsp_adc_map[ch].adc == &hadc1 && adc_calibrated[0] == 0) {
    adc_calibrated[0] = 1;
  } else if (bsp_adc_map[ch].adc == &hadc2 && adc_calibrated[1] == 0) {
    adc_calibrated[1] = 1;
  } else {
    return BSP_ERR; // Repeatedly calibration error or Channel error
  }

  HAL_ADCEx_Calibration_Start(bsp_adc_map[ch].adc, ADC_SINGLE_ENDED);

  return BSP_OK;
}

/**
 * @brief ADC start on DMA mode
 *        only need be down twice here (adc1/2)
 * @param ch
 * @return bsp_status_t
 */
bsp_status_t bsp_adc_start(bsp_adc_channel_t ch) {

  static adc_running[2] = {0};

  if (bsp_adc_map[ch].adc == &hadc1 && adc_running[0] == 0) {
    adc_running[0] = 1;
    HAL_ADC_Start_DMA(bsp_adc_map[ch].adc, adcBuf[0], ADC_BUFFER_SIZE);
  } else if (bsp_adc_map[ch].adc == &hadc2 && adc_running[1] == 0) {
    adc_running[1] = 1;
    HAL_ADC_Start_DMA(bsp_adc_map[ch].adc, adcBuf[1], ADC_BUFFER_SIZE);
  } else {
    return BSP_ERR; // Repeatedly start error or Channel error
  }

  return BSP_OK;
}
/**
 * @brief Get SUM of ADC_SAMPLE_COUNT times adc value
 *
 * @param ch
 * @return bsp_status_t
           the result will be drop into bsp_adc_sumBuf[ch]
 */
bsp_status_t BSP_ADC_GetSumBuffer(bsp_adc_channel_t ch) {

  if (ch >= BSP_ADC_NUM) {
    return BSP_ERR; // Channel error
  }

  for (uint16_t i = bsp_adc_map[ch].offset; i < ADC_BUFFER_SIZE;
       i += ADC_CHANNEL_COUNT) {
    bsp_adc_sumBuf[ch] += adcBuf[i];
  }

  return BSP_OK;
}
