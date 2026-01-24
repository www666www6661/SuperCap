#include "mod_powerctrl.h"
#include "comp_pid.h"
#include "comp_utils.h"
#include "dev_buckboost.h"
#include "mod_status.h"
#include <math.h>

void Module_PowerCtrl_Init(Module_PowerCtrl *this,
                           Module_PowerCtrl_Param param) {

  this->param_ = param;

  this->dt = this->param_.dt;
  this->base_referee_power_ = this->param_.default_base_referee_power;

  this->sampler_ = this->param_.sampler_;
  this->status_ = this->param_.status_;

  // TODO:连接C板，获取上位机参数
  this->pRefree_setpoint_ = this->param_.default_base_referee_power;
  this->status_->chassisPowerLimit = this->pRefree_setpoint_;

  Component_PID_Init(&(this->PID_vbside_), this->param_.vbside);
  Component_PID_Init(&(this->PID_iaside_), this->param_.iaside);
  Component_PID_Init(&(this->PID_pRefree_), this->param_.preferee);
  Component_PID_Init(&(this->PID_energy_), this->param_.energy);
  Device_BuckBoost_Init(&(this->buckboost_), this->param_.buckboost);
  Device_BuckBoost_Enable();
}

volatile float temp;

void Module_PowerCtrl_Control(Module_PowerCtrl *this) {
  if (this->sampler_->vaside_.voltage_ > this->buckboost_.BAT_VOLTAGE_MIN) {

    // TODO: 把这个完善
    float actual_ia_to_ib = (MAX(ABS(this->sampler_->iaside_.current_), 0.1f)) /
                            (MAX(ABS(this->sampler_->ibside_.current_), 0.1f));

    this->paside_setpoint_ = Component_PID_Calculate(
        &(this->PID_pRefree_), this->pRefree_setpoint_,
        this->sampler_->vaside_.voltage_ * this->sampler_->iRefree_.current_,
        this->dt);

    temp = this->sampler_->vaside_.voltage_ * this->sampler_->iRefree_.current_;

    /**
     * @brief 电流limit目标控制
     * 电压偏大(only out):等于CAP_IOUT_MAX
     * 偏小(out and in):等于CAP_IOUT_MIN
     * 电容组电压在NORMAL-MIN区间(only out)内等于:
     * 一个偏置为CAP_IOUT_MIN，斜率为CAP_IOUT_MAX-CAP_IOUT_MIN,
     * 变量为电容(voltage - CAP_CUTOFF_VOLTAGE)/
     * (CAP_NORMAL_VOLTAGE-CAP_CUTOFF_VOLTAGE)的直线
     *
     */
    /*====================================================================================*/
    float temp_cap_out_ilimit;
    float temp_cap_in_ilimit = this->buckboost_.I_LIMIT;
    if (this->sampler_->vbside_.voltage_ <
        this->buckboost_.CAP_CUTOFF_VOLTAGE) // 电容组电压偏小
    {
      temp_cap_out_ilimit = this->buckboost_.CAP_IOUT_MIN;
      temp_cap_in_ilimit = 4.0f;
    } else if (this->sampler_->vbside_.voltage_ >
               this->buckboost_.CAP_NORMAL_VOLTAGE) // 电容组电压大于NORMAL
    {
      temp_cap_out_ilimit = this->buckboost_.CAP_IOUT_MAX;
    } else {
      temp_cap_out_ilimit =
          this->buckboost_.CAP_IOUT_MIN +
          (this->buckboost_.CAP_IOUT_MAX - this->buckboost_.CAP_IOUT_MIN) *
              (this->sampler_->vbside_.voltage_ -
               this->buckboost_.CAP_CUTOFF_VOLTAGE) /
              (this->buckboost_.CAP_NORMAL_VOLTAGE -
               this->buckboost_.CAP_CUTOFF_VOLTAGE);
      clampf(&temp_cap_out_ilimit, this->buckboost_.CAP_IOUT_MIN,
             this->buckboost_.CAP_IOUT_MAX);
    }
    /*====================================================================================*/
    float power_limit_a_to_b =
        MIN(this->buckboost_.I_LIMIT * this->sampler_->vaside_.voltage_,
            temp_cap_in_ilimit * this->sampler_->vaside_.voltage_ *
                actual_ia_to_ib);
    float power_limit_b_to_a =
        MAX(-1 * this->buckboost_.I_LIMIT * this->sampler_->vaside_.voltage_,
            -1 * temp_cap_out_ilimit * this->sampler_->vaside_.voltage_ *
                actual_ia_to_ib);
    if (this->paside_setpoint_ < power_limit_b_to_a) {
      this->paside_setpoint_ = power_limit_b_to_a;
      if (this->base_referee_power_ > this->status_->chassisPowerLimit + 3.0f)
        this->base_referee_power_ = this->status_->chassisPowerLimit + 3.0f;
    } else if (this->paside_setpoint_ > power_limit_a_to_b) {
      this->paside_setpoint_ = power_limit_a_to_b;
      if (this->base_referee_power_ > this->status_->chassisPowerLimit + 3.0f)
        this->base_referee_power_ = this->status_->chassisPowerLimit + 3.0f;
    }

    // // TODO:在完整逻辑改出之后视情况删掉这里
    // if (this->sampler_->vaside_.voltage_ > this->buckboost_.BAT_VOLTAGE_MIN)
    // {
    //   this->iaside_setpoint_ =
    //       this->paside_setpoint_ / this->sampler_->vaside_.voltage_;
    // } else {
    //   this->iaside_setpoint_ = 0.0f; // 电压不足时，不拉电流
    // }

    this->iaside_setpoint_ = this->iaside_setpoint_ > 1.0f
                                 ? 0.4f
                                 : this->iaside_setpoint_ + 0.00005f;

    /*电压前馈*/
    float ff_voltage_ratio =
        this->sampler_->vbside_.voltage_ / this->sampler_->vaside_.voltage_;

    /* 伏秒平衡推导的电压前馈 + 电流前馈*/
    float ff_ratio =
        ff_voltage_ratio + this->param_.k_feedforward * this->iaside_setpoint_;

    float ia_duty =
        Component_PID_Calculate(&(this->PID_iaside_), this->iaside_setpoint_,
                                this->sampler_->iaside_.current_, this->dt) +
        ff_ratio; // 前馈加上pid

    if (this->sampler_->vbside_.voltage_ >
        this->buckboost_.CAP_MAX_VOLTAGE * 0.9f) {
      float vb_duty =
          Component_PID_Calculate(&(this->PID_vbside_),
                                  this->buckboost_.CAP_MAX_VOLTAGE,
                                  this->sampler_->vbside_.voltage_, this->dt) +
          ff_voltage_ratio;
      if (vb_duty < ia_duty) {
        this->output_duty_ = vb_duty;
        if (this->base_referee_power_ > this->status_->chassisPowerLimit + 3.0f)
          this->base_referee_power_ = this->status_->chassisPowerLimit + 3.0f;
        if (this->paside_setpoint_ > this->sampler_->ibside_.current_ *
                                             this->buckboost_.CAP_MAX_VOLTAGE +
                                         8.0f)
          this->paside_setpoint_ = this->sampler_->ibside_.current_ *
                                       this->buckboost_.CAP_MAX_VOLTAGE +
                                   8.0f;
      } else {
        this->output_duty_ = ia_duty;
      }
    } else {
      Component_PID_Reset(&(this->PID_vbside_));
      this->output_duty_ = ia_duty;
    }
    this->status_->outputduty = this->output_duty_;

    clampf(&(this->output_duty_), 0.05f, 10.0f);

  } else {
    this->output_duty_ =
        this->sampler_->vbside_.voltage_ / this->sampler_->vaside_.voltage_;
    this->paside_setpoint_ = 0.0f;
    this->iaside_setpoint_ = 0.0f;
    clampf(&(this->output_duty_), 0.05f, 10.0f);

    Component_PID_Reset(&(this->PID_iaside_));
    Component_PID_Reset(&(this->PID_vbside_));
    Component_PID_Reset(&(this->PID_pRefree_));
  }

  Device_BuckBoost_UpdatePWM(this->output_duty_);
}
