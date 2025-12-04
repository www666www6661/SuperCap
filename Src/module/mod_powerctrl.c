#include "mod_powerctrl.h"
#include "comp_pid.h"
#include "comp_utils.h"
#include "dev_buckboost.h" // For updatePWM
#include "mod_status.h"
#include <math.h>

void Module_PowerCtrl_Init(Module_PowerCtrl *this,
                           Module_PowerCtrl_Param param) {

  this->param_ = param;

  Component_PID_Init(&(this->Component_PID_vbside_), this->param_.vbside);
  Component_PID_Init(&(this->Component_PID_iaside_), this->param_.iaside);
  Component_PID_Init(&(this->Component_PID_prefree_), this->param_.preferee);
  Component_PID_Init(&(this->Component_PID_energy_), this->param_.energy);
}

void Module_PowerCtrl_Update(Module_PowerCtrl *this) {
#ifndef CALIBRATION_MODE
  if (module_status.outputEnabled) {
    float dt = 0.001f; // Placeholder for time delta
    float tempVASide = this->vaside_.voltage_;

    float absIA = MIN(fabsf(this->Iaside_.current_), 0.1f);
    float absIB = MIN(fabsf(this->Ibside_.current_), 0.1f);
    float actualIAtoIB = absIA / absIB;

    float pReferee_fb = this->Vaside_.voltage_ * this->IRefree_.current_;
    float pReferee_out = Component_PID_Calculate(
        &pidPowerReferee, module_powerctrl_tempdata.targetRefereePower,
        pReferee_fb, dt);
    module_powerctrl_tempdata.targetAPower += pReferee_out;

    float tempCapOutILimit;
    float tempCapInILimit = I_LIMIT;
    if (this->Vbside_.voltage_ < this->param_.cap_v_cutoff) {
      tempCapOutILimit = this->param_.min_cap_iout;
      tempCapInILimit = 4.0f;
    } else if (this->Vbside_.voltage_ > this->param_.cap_v_normal) {
      tempCapOutILimit = this->param_.max_cap_iout;
    } else {
      tempCapOutILimit =
          this->param_.min_cap_iout +
          (this->param_.max_cap_iout - this->param_.min_cap_iout) *
              (this->Vbside_.voltage_ - this->param_.cap_v_cutoff) /
              (this->param_.cap_v_normal - this->param_.cap_v_cutoff);
      clampf(&tempCapOutILimit, this->param_.min_cap_iout,
             this->param_.max_cap_iout);
    }

    float powerALimitNegativeBoundByA = -1 * I_LIMIT * tempVASide;
    float powerALimitPositiveBoundByA = I_LIMIT * tempVASide;

    float powerALimitNegativeBoundByB =
        -1 * tempCapOutILimit * tempVASide * actualIAtoIB;
    float powerALimitPositiveBoundByB =
        tempCapInILimit * tempVASide * actualIAtoIB;

    float powerLimitAToB =
        MIN(powerALimitPositiveBoundByA, powerALimitPositiveBoundByB);
    float powerLimitBToA =
        MAX(powerALimitNegativeBoundByA, powerALimitNegativeBoundByB);

    if (module_powerctrl_tempdata.targetAPower < powerLimitBToA) {
      module_powerctrl_tempdata.targetAPower = powerLimitBToA;
      if (module_powerctrl_tempdata.baseRefereePower >
          module_powerctrl_controldata.refereePowerLimit + 3.0f)
        module_powerctrl_tempdata.baseRefereePower =
            module_powerctrl_controldata.refereePowerLimit + 3.0f;
    } else if (module_powerctrl_tempdata.targetAPower > powerLimitAToB) {
      module_powerctrl_tempdata.targetAPower = powerLimitAToB;
      if (module_powerctrl_tempdata.baseRefereePower >
          module_powerctrl_controldata.refereePowerLimit + 3.0f)
        module_powerctrl_tempdata.baseRefereePower =
            module_powerctrl_controldata.refereePowerLimit + 3.0f;
    }

    module_powerctrl_tempdata.targetIA =
        module_powerctrl_tempdata.targetAPower / tempVASide;

    float currentA_out = Component_PID_Calculate(
        &pidCurrentA, module_powerctrl_tempdata.targetIA,
        this->Iaside_.current_, dt);

    static float extraErrorSum = 0;
    static float kII = 1.1e-05f;
    static float iiDecay = 0.988f;
    if (kII != 0) {
      extraErrorSum =
          extraErrorSum * iiDecay +
          (module_powerctrl_tempdata.targetIA - this->Iaside_.current_);
      clampf(&extraErrorSum, -500, 500);
    }

    float iaDuty = module_powerctrl_tempdata.outputDuty +
                   (currentA_out + kII * extraErrorSum);

    if (this->Vbside_.voltage_ > MAX_CAP_VOLTAGE * 0.9f) {
      float voltageB_out = Component_PID_Calculate(
          &pidVoltageB, MAX_CAP_VOLTAGE, this->Vbside_.voltage_, dt);

      float vbDuty =
          (module_powerctrl_tempdata.outputDuty * this->Vaside_.voltage_ +
           voltageB_out) /
          this->Vaside_.voltage_;
      if (vbDuty < iaDuty) {
        module_powerctrl_tempdata.outputDuty = vbDuty;
        extraErrorSum = 0;
        if (module_powerctrl_tempdata.baseRefereePower >
            module_powerctrl_controldata.refereePowerLimit + 3.0f)
          module_powerctrl_tempdata.baseRefereePower =
              module_powerctrl_controldata.refereePowerLimit + 3.0f;
        if (module_powerctrl_tempdata.targetAPower >
            this->Ibside_.current_ * MAX_CAP_VOLTAGE + 8.0f)
          module_powerctrl_tempdata.targetAPower =
              this->Ibside_.current_ * MAX_CAP_VOLTAGE + 8.0f;
      } else
        module_powerctrl_tempdata.outputDuty = iaDuty;
    } else {
      Component_PID_Calculate(&pidVoltageB, MAX_CAP_VOLTAGE,
                              this->Vbside_.voltage_, dt);
      module_powerctrl_tempdata.outputDuty = iaDuty;
    }

    clampf(&module_powerctrl_tempdata.outputDuty, 0.05f, 10.0f);
  } else {
    module_powerctrl_tempdata.outputDuty =
        this->Vbside_.voltage_ / this->Vaside_.voltage_;
    module_powerctrl_tempdata.targetAPower = 0.0f;
    module_powerctrl_tempdata.targetIA = 0.0f;
    clampf(&module_powerctrl_tempdata.outputDuty, 0.05f, 10.0f);

    float dt = 0.001f; // Placeholder for time delta
    Component_PID_Calculate(&pidCurrentA, 0, this->Iaside_.current_, dt);
    Component_PID_Calculate(&pidVoltageB, MAX_CAP_VOLTAGE,
                            this->Vbside_.voltage_, dt);
    float pReferee_fb = this->Vaside_.voltage_ * this->IRefree_.current_;
    Component_PID_Calculate(&pidPowerReferee,
                            module_powerctrl_tempdata.targetRefereePower,
                            pReferee_fb, dt);
  }
#endif
  Device_BuckBoost_UpdatePWM(module_powerctrl_tempdata.outputDuty);
}
