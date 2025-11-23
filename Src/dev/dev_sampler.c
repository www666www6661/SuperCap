#include "dev_sampler.h"
#include "bsp_adc.h"
#include "stm32f3xx_hal.h"

void Device_Volt_Sampler_Init(Device_Volt_Sampler *this,
                              Device_Sampler_Param param) {
  this->param_ = param;
  bsp_adc_start(this->param_.adc_channel);
  this->voltage_ = 0;
}

void Device_Current_Sampler_Init(Device_Current_Sampler *this,
                                 Device_Sampler_Param param) {
  this->param_ = param;
  bsp_adc_start(this->param_.adc_channel);
  this->current_ = 0;
}

float Device_Volt_GetValue(Device_Volt_Sampler *this) {
  bsp_adc_updatesumbuf(this->param_.adc_channel);
  bsp_adc_dumpdata(this->param_.adc_channel, &(this->adc_val_));
  return this->voltage_ = this->param_.k * this->adc_val_ + this->param_.b;
}

float Device_Current_GetValue(Device_Current_Sampler *this) {
  bsp_adc_updatesumbuf(this->param_.adc_channel);
  bsp_adc_dumpdata(this->param_.adc_channel, &(this->adc_val_));
  return this->current_ = this->param_.k * this->adc_val_ + this->param_.b;
}
