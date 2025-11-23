#include "comp_pid.h"
#include "SuperCap.h"
#include "comp_utils.h"
#include <math.h>
#include <string.h>

#define SIGMA 0.000001f

void Component_PID_Init(Component_PID *pid, Component_PID_Param param) {
  memset(&pid->dfilter_, 0, sizeof(pid->dfilter_));
  memset(&pid->last_, 0, sizeof(pid->last_));
  memset(&pid->dt_min_, 0, sizeof(pid->dt_min_));
  memset(&pid->i_, 0, sizeof(pid->i_));

  pid->param_ = param;

  LowPassFilter_Init(&pid->dfilter_, pid->param_.d_cutoff_freq);
}

/**
 * @brief Calculates the PID controller output.
 * @param this  Pointer to the PID controller instance.
 * @param sp    The setpoint (desired value).
 * @param fb    The feedback (measured value).
 * @param dt    The time interval (delta time) in seconds.
 * @return float The calculated controller output.
 * @note This implementation uses derivative on measurement to prevent
 * "derivative kick" and includes integral anti-windup. The derivative term is
 * also low-pass filtered.
 */
float Component_PID_Calculate(Component_PID *this, float sp, float fb,
                              float dt) {
  if (!isfinite(sp) || !isfinite(fb) || !isfinite(dt)) {
    return this->last_.out;
  }

  /* 计算误差值 */
  float err = sp - fb;

  /* 计算P项 */
  float k_err = err * this->param_.k;

  /* 计算D项 */
  const float K_FB = this->param_.k * fb;
  const float FILTERED_K_FB = LowPassFilter_Apply(&this->dfilter_, K_FB, dt);

  /* 通过fb计算D，避免了由于sp变化导致err突变的问题 */
  /* 当sp不变时，err的微分等于负的fb的微分 */
  float d = (FILTERED_K_FB - this->last_.k_fb) / fmaxf(dt, this->dt_min_);

  this->last_.err = err;
  this->last_.k_fb = FILTERED_K_FB;

  if (!isfinite(d)) {
    d = 0.0f;
  }
  /* 计算PD输出 */
  float output = (k_err * this->param_.p) - (d * this->param_.d);

  /* 计算I项 */
  const float I = this->i_ + (k_err * dt);
  const float I_OUT = I * this->param_.i;

  if (this->param_.i > SIGMA) {
    /* 检查是否饱和 */
    if (isfinite(I)) {
      if ((fabsf(output + I_OUT) <= this->param_.out_limit) &&
          (fabsf(I) <= this->param_.i_limit)) {
        /* 未饱和，使用新积分 */
        this->i_ = I;
      }
    }
  }

  /* 计算PID输出 */
  output += I_OUT;

  /* 限制输出 */
  if (isfinite(output)) {
    if (this->param_.out_limit > SIGMA) {
      output = abs_clampf(output, this->param_.out_limit);
    }
    this->last_.out = output;
  }
  return this->last_.out;
}

void Component_PID_SetK(Component_PID *this, float k) { this->param_.k = k; }

void Component_PID_SetP(Component_PID *this, float p) { this->param_.p = p; }

void Component_PID_SetI(Component_PID *this, float i) { this->param_.i = i; }

void Component_PID_SetD(Component_PID *this, float d) { this->param_.d = d; }

void Component_PID_Reset(Component_PID *this) {

  this->i_ = 0.0f;
  this->last_.err = 0.0f;
  this->last_.k_fb = 0.0f;
  this->last_.out = 0.0f;
  LowPassFilter_Reset(&this->dfilter_, 0.0f);
}
