#pragma once

#include "dev_buzzer.h"
#include "dev_sampler.h"

void SuperCap_Start();

struct status {
  float vAside;
  float vBside;

  float iAside;
  float iBside;
  float iReferee;
  float iChassis;

  float pAside;
  float pBside;

  float efficience;
  float outputduty;
  float temprature;
  float dcdc_mode; // 以电池 -> 电容作为参考方向
};
