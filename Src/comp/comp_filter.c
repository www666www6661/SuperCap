/*
  各类滤波器。
*/

#include "comp_filter.h"
#include <math.h>

void LowPassFilter_Init(LowPassFilter *lpf, float cut_freq) {
  lpf->cut_freq_ = cut_freq;
  lpf->last_out_ = 0;
}

float LowPassFilter_Apply(LowPassFilter *lpf, float sample, float dt) {
  float k = 2 * M_2PI * lpf->cut_freq_ * dt;
  k = k / (1 + k);
  float out = k * sample + (1 - k) * lpf->last_out_;
  lpf->last_out_ = out;

  return out;
}

void LowPassFilter_Reset(LowPassFilter *lpf, float sample) {
  lpf->last_out_ = sample;
}

void LowPassFilter2p_Init(LowPassFilter2p *lpf, float sample_freq,
                          float cutoff_freq) {
  lpf->cutoff_freq_ = cutoff_freq;
  lpf->delay_element_1_ = 0.0f;
  lpf->delay_element_2_ = 0.0f;

  if (lpf->cutoff_freq_ <= 0.0f) {
    /* no filtering */
    lpf->b0_ = 1.0f;
    lpf->b1_ = 0.0f;
    lpf->b2_ = 0.0f;

    lpf->a1_ = 0.0f;
    lpf->a2_ = 0.0f;

    return;
  }
  const float FR = sample_freq / lpf->cutoff_freq_;
  const float OHM = tanf(M_PI / FR);
  const float C = 1.0f + 2.0f * cosf(M_PI / 4.0f) * OHM + OHM * OHM;

  lpf->b0_ = OHM * OHM / C;
  lpf->b1_ = 2.0f * lpf->b0_;
  lpf->b2_ = lpf->b0_;

  lpf->a1_ = 2.0f * (OHM * OHM - 1.0f) / C;
  lpf->a2_ = (1.0f - 2.0f * cosf(M_PI / 4.0f) * OHM + OHM * OHM) / C;
}

float LowPassFilter2p_Apply(LowPassFilter2p *lpf, float sample) {
  /* do the filtering */
  float delay_element_0 = sample - lpf->delay_element_1_ * lpf->a1_ -
                          lpf->delay_element_2_ * lpf->a2_;

  if (isinf(delay_element_0)) {
    /* don't allow bad values to propagate via the filter */
    delay_element_0 = sample;
  }

  const float OUTPUT = delay_element_0 * lpf->b0_ +
                       lpf->delay_element_1_ * lpf->b1_ +
                       lpf->delay_element_2_ * lpf->b2_;

  lpf->delay_element_2_ = lpf->delay_element_1_;
  lpf->delay_element_1_ = delay_element_0;

  /* return the value. Should be no need to check limits */
  return OUTPUT;
}

float LowPassFilter2p_Reset(LowPassFilter2p *lpf, float sample) {
  const float DVAL = sample / (lpf->b0_ + lpf->b1_ + lpf->b2_);

  if (isfinite(DVAL)) {
    lpf->delay_element_1_ = DVAL;
    lpf->delay_element_2_ = DVAL;

  } else {
    lpf->delay_element_1_ = sample;
    lpf->delay_element_2_ = sample;
  }

  return LowPassFilter2p_Apply(lpf, sample);
}
