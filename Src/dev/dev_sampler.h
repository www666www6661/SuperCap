/**
 * @file dev_sampler.h
 * @author concon
 * @brief"adc计算得出的数据往往与真实数据之间存在误差，而且基本表现为线性误差"--ENTERPRIZE_RM2024-SuperCap-开源报告。
 *        所以直接将adc_sum与真实数据直接使用一个线性方程转换,
 *        即 voltage_/currrent_ = k * sum_ + b
 * @version 0.1
 * @date 2025-11-18
 *
 *
 */
#pragma once

#include "bsp_adc.h"

#define SUM

typedef struct {
  bsp_adc_channel_t adc_channel;
  float k;
  float b;
} Device_Sampler_Param;

typedef struct {
  Device_Sampler_Param param_;
  uint16_t adc_val_;
  float voltage_;
} Device_Volt_Sampler;

typedef struct {
  Device_Sampler_Param param_;
  uint16_t adc_val_;
  float current_;
} Device_Current_Sampler;

/**
 * @brief Initializes the voltage sampler.
 * @param this Pointer to the Device_Volt_Sampler instance.
 * @param param Initialization parameters.
 */
static inline void Device_Volt_Sampler_Init(Device_Volt_Sampler *this,
                                            Device_Sampler_Param param) {
  this->param_ = param;
  bsp_adc_start(this->param_.adc_channel);
  this->voltage_ = 0;
}

/**
 * @brief Initializes the current sampler.
 * @param this Pointer to the Device_Current_Sampler instance.
 * @param param Initialization parameters.
 */
static inline void Device_Current_Sampler_Init(Device_Current_Sampler *this,
                                               Device_Sampler_Param param) {
  this->param_ = param;
  bsp_adc_start(this->param_.adc_channel);
  this->current_ = 0;
}

/**
 * @brief Gets the latest voltage value.
 * @param this Pointer to the Device_Volt_Sampler instance.
 * @return The calculated voltage value.
 */
static inline float Device_Sampler_GetVoltage(Device_Volt_Sampler *this) {
  bsp_adc_updatesumbuf(this->param_.adc_channel);
  bsp_adc_dumpdata(this->param_.adc_channel, &(this->adc_val_));
  return this->voltage_ = this->param_.k * this->adc_val_ + this->param_.b;
}

/**
 * @brief Gets the latest current value.
 * @param this Pointer to the Device_Current_Sampler instance.
 * @return The calculated current value.
 */
static inline float Device_Sampler_GetCurrrent(Device_Current_Sampler *this) {
  bsp_adc_updatesumbuf(this->param_.adc_channel);
  bsp_adc_dumpdata(this->param_.adc_channel, &(this->adc_val_));
  return this->current_ = this->param_.k * this->adc_val_ + this->param_.b;
}
