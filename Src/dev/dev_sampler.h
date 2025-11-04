#pragma once

#include "bsp_adc.h"

typedef struct {
  bsp_adc_channel_t adc_channel;
  float mult;   // 放大器倍数
  float resist; // 采样电阻大小
} Device_Sampler_Param;

typedef struct {
  Device_Sampler_Param param_;
  float voltage_;
} Device_Volt_Sampler;

typedef struct {
  Device_Sampler_Param param_;
  float current_;
} Device_Current_Sampler;

void Device_Volt_Sampler_Init(Device_Volt_Sampler *this);

void Device_Current_Sampler_Init(Device_Volt_Sampler *this);