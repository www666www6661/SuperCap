/*
  各类滤波器。
*/

#pragma once

#include "component.h"

#ifndef M_2PI
#define M_2PI 6.28318530717958647692f
#endif

/* 一阶数字低通滤波器 */
typedef struct {
  float cut_freq_;
  float last_out_;
} LowPassFilter;

void LowPassFilter_Init(LowPassFilter *lpf, float cut_freq);
float LowPassFilter_Apply(LowPassFilter *lpf, float sample, float dt);
void LowPassFilter_Reset(LowPassFilter *lpf, float sample);

/* 二阶巴特沃斯低通滤波器 */
typedef struct {
  float cutoff_freq_; /* 截止频率 */

  float a1_;
  float a2_;

  float b0_;
  float b1_;
  float b2_;

  float delay_element_1_;
  float delay_element_2_;
} LowPassFilter2p;

void LowPassFilter2p_Init(LowPassFilter2p *lpf, float sample_freq,
                          float cutoff_freq);
float LowPassFilter2p_Apply(LowPassFilter2p *lpf, float sample);
float LowPassFilter2p_Reset(LowPassFilter2p *lpf, float sample);
