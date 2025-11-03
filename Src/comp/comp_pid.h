#pragma once

#include "comp_filter.h"
#include "component.h"

/* PID参数 */
typedef struct Param {
  float k;             /* 控制器增益，设置为1用于并行模式 */
  float p;             /* 比例项增益，设置为1用于标准形式 */
  float i;             /* 积分项增益 */
  float d;             /* 微分项增益 */
  float i_limit;       /* 积分项上限 */
  float out_limit;     /* 输出绝对值限制 */
  float d_cutoff_freq; /* D项低通截止频率 */
  bool cycle;          /* 是否为循环角度值 */
} Component_PID_Param;

typedef struct Last {
  float err;  /* 上次误差 */
  float k_fb; /* 上次反馈值 */
  float out;  /* 上次输出 */
} Component_PID_Last;

typedef struct Component_PID {
  Component_PID_Param param_;
  Component_PID_Last last_;
  float dt_min_;          /* 最小PID_Calc调用间隔 */
  float i_;               /* 积分 */
  LowPassFilter dfilter_; /* D项低通滤波器 */
} Component_PID;

void PID_Init(Component_PID *pid, Component_PID_Param param_);