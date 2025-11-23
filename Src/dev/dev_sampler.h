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

void Device_Volt_Sampler_Init(Device_Volt_Sampler *this,
                              Device_Sampler_Param param);
void Device_Current_Sampler_Init(Device_Current_Sampler *this,
                                 Device_Sampler_Param param);
float Device_Volt_GetValue(Device_Volt_Sampler *this);
float Device_Current_GetValue(Device_Current_Sampler *this);
