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

  Component_PID_Init(&(this->PID_vbside_), this->param_.vbside);
  Component_PID_Init(&(this->PID_iaside_), this->param_.iaside);
  Component_PID_Init(&(this->PID_pRefree_), this->param_.preferee);
  Component_PID_Init(&(this->PID_energy_), this->param_.energy);
  Device_BuckBoost_Init(&(this->buckboost_), this->param_.buckboost);
  Device_BuckBoost_Enable();
}

void Module_PowerCtrl_Calculate(Module_PowerCtrl *this) {
  if (this->status_->outputEnabled) {
    // TODO: 把这个完善
    float actual_ia_to_ib =
        (fminf(fabsf(this->sampler_->iaside_.current_), 0.1f)) /
        (fminf(fabsf(this->sampler_->ibside_.current_), 0.1f));

    this->paside_setpoint_ = Component_PID_Calculate(
        &(this->PID_pRefree_), this->pRefree_setpoint_,
        this->sampler_->vaside_.voltage_ * this->sampler_->iRefree_.current_,
        this->dt);

    float temp_cap_out_ilimit;
    float temp_cap_in_ilimit = this->buckboost_.I_LIMIT;
    if (this->sampler_->vbside_.voltage_ <
        this->buckboost_.CAP_CUTOFF_VOLTAGE) {
      temp_cap_out_ilimit = this->buckboost_.CAP_IOUT_MIN;
      temp_cap_in_ilimit = 4.0f;
    } else if (this->sampler_->vbside_.voltage_ >
               this->buckboost_.CAP_NORMAL_VOLTAGE) {
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

    float power_limit_a_to_b =
        fminf(this->buckboost_.I_LIMIT * this->sampler_->vaside_.voltage_,
              temp_cap_in_ilimit * this->sampler_->vaside_.voltage_ *
                  actual_ia_to_ib);
    float power_limit_b_to_a =
        fmaxf(-1 * this->buckboost_.I_LIMIT * this->sampler_->vaside_.voltage_,
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

    this->iaside_setpoint_ =
        this->paside_setpoint_ / this->sampler_->vaside_.voltage_;

    float ia_duty =
        Component_PID_Calculate(&(this->PID_iaside_), this->iaside_setpoint_,
                                this->sampler_->iaside_.current_, this->dt);

    if (this->sampler_->vbside_.voltage_ >
        this->buckboost_.CAP_MAX_VOLTAGE * 0.9f) {
      float vb_duty = Component_PID_Calculate(
          &(this->PID_vbside_), this->buckboost_.CAP_MAX_VOLTAGE,
          this->sampler_->vbside_.voltage_, this->dt);

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
  Device_BuckBoost_UpdatePWM(0.001f);
}
