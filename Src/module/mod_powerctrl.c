#include "mod_powerctrl.h"

#include <math.h>

#include "comp_pid.h"
#include "comp_utils.h"
#include "dev_buckboost.h"
#include "mod_status.h"

void Module_PowerCtrl_Init(Module_PowerCtrl *this, Module_PowerCtrl_Param param)
{
    this->param_ = param;

    this->dt = this->param_.dt;
    this->base_referee_power_ = this->param_.default_base_referee_power;

    this->sampler_ = this->param_.sampler_;
    this->status_ = this->param_.status_;

    this->pRefree_setpoint_ = this->param_.default_base_referee_power;
    this->status_->chassisPowerLimit = this->pRefree_setpoint_;
    this->buckboost_mode_ = BUCK;

    Component_PID_Init(&(this->PID_vbside_), this->param_.vbside);
    Component_PID_Init(&(this->PID_pRefree_), this->param_.preferee);

    Component_PID_Init(&(this->PID_ialpha_), this->param_.ialpha);
    Component_PID_Init(&(this->PID_ibeta_), this->param_.ibeta);
    Component_PID_Init(&(this->PID_igamma_), this->param_.igamma);

    param.buckboost.phase = PHAS_ALPHA;
    Device_BuckBoost_Init(&(this->buckboost_alpha_), param.buckboost);

    param.buckboost.phase = PHAS_BETA;
    Device_BuckBoost_Init(&(this->buckboost_beta_), param.buckboost);

    param.buckboost.phase = PHAS_GAMMA;
    Device_BuckBoost_Init(&(this->buckboost_gamma_), param.buckboost);

    Device_BuckBoost_Enable();
}

volatile float temp;

void Module_PowerCtrl_Control(Module_PowerCtrl *this)
{
    if (this->sampler_->vaside_.voltage_ > this->buckboost_alpha_.BAT_VOLTAGE_MIN)
    {
        // =========================================================================
        float VbtoVa = MAX(this->sampler_->vbside_.voltage_, 0.1f) / MAX(this->sampler_->vaside_.voltage_, 0.1f);

        this->paside_setpoint_ = Component_PID_Calculate(
            &(this->PID_pRefree_), this->pRefree_setpoint_, this->sampler_->vaside_.voltage_ * this->sampler_->iRefree_.current_, this->dt);

        temp = this->sampler_->vaside_.voltage_ * this->sampler_->iRefree_.current_;

        float temp_cap_out_ilimit;
        float temp_cap_in_ilimit = this->buckboost_alpha_.I_LIMIT;
        if (this->sampler_->vbside_.voltage_ < this->buckboost_alpha_.CAP_CUTOFF_VOLTAGE)
        {
            temp_cap_out_ilimit = this->buckboost_alpha_.CAP_IOUT_MIN;
            temp_cap_in_ilimit = 4.0f;
        }
        else if (this->sampler_->vbside_.voltage_ > this->buckboost_alpha_.CAP_NORMAL_VOLTAGE)
        {
            temp_cap_out_ilimit = this->buckboost_alpha_.CAP_IOUT_MAX;
        }
        else
        {
            temp_cap_out_ilimit =
                this->buckboost_alpha_.CAP_IOUT_MIN + (this->buckboost_alpha_.CAP_IOUT_MAX - this->buckboost_alpha_.CAP_IOUT_MIN) *
                                                          (this->sampler_->vbside_.voltage_ - this->buckboost_alpha_.CAP_CUTOFF_VOLTAGE) /
                                                          (this->buckboost_alpha_.CAP_NORMAL_VOLTAGE - this->buckboost_alpha_.CAP_CUTOFF_VOLTAGE);
            clampf(&temp_cap_out_ilimit, this->buckboost_alpha_.CAP_IOUT_MIN, this->buckboost_alpha_.CAP_IOUT_MAX);
        }

        float power_limit_a_to_b =
            MIN(this->buckboost_alpha_.I_LIMIT * this->sampler_->vaside_.voltage_, temp_cap_in_ilimit * this->sampler_->vaside_.voltage_ * VbtoVa);
        float power_limit_b_to_a = MAX(-1 * this->buckboost_alpha_.I_LIMIT * this->sampler_->vaside_.voltage_,
                                       -1 * temp_cap_out_ilimit * this->sampler_->vaside_.voltage_ * VbtoVa);

        if (this->paside_setpoint_ < power_limit_b_to_a)
        {
            this->paside_setpoint_ = power_limit_b_to_a;
            if (this->base_referee_power_ > this->status_->chassisPowerLimit + 3.0f)
                this->base_referee_power_ = this->status_->chassisPowerLimit + 3.0f;
        }
        else if (this->paside_setpoint_ > power_limit_a_to_b)
        {
            this->paside_setpoint_ = power_limit_a_to_b;
            if (this->base_referee_power_ > this->status_->chassisPowerLimit + 3.0f)
                this->base_referee_power_ = this->status_->chassisPowerLimit + 3.0f;
        }

        this->iaside_setpoint_ = abs_clampf(this->paside_setpoint_ / MAX(this->sampler_->vaside_.voltage_, 0.1f), this->buckboost_alpha_.I_LIMIT);

        // =========================================================================

        float k_phase_map = 1.0f;
        if (VbtoVa <= 0.97f)
        {
            k_phase_map = 1.0f / VbtoVa;
        }
        else if (VbtoVa >= 1.03f)
        {
            k_phase_map = 1.0f;
        }
        else
        {
            float k_buck = 1.0f / VbtoVa;
            float blend = (VbtoVa - 0.97f) / (1.03f - 0.97f);
            clampf(&blend, 0.0f, 1.0f);
            k_phase_map = k_buck + blend * (1.0f - k_buck);
        }

        this->iphase_setpoint_ = k_phase_map * this->iaside_setpoint_;

        // 前馈比率 (V_out / V_in)
        float volt_feedforward = VbtoVa;

        // 计算三相均流参考
        float ibase_setpoint = this->iphase_setpoint_ / 3.0f;
        float iavg = (this->sampler_->ialpha_.current_ + this->sampler_->ibeta_.current_ + this->sampler_->igamma_.current_) / 3.0f;

        float ialpha_share = abs_clampf(this->param_.share_gain * (iavg - this->sampler_->ialpha_.current_), this->param_.share_limit);
        float ibeta_share = abs_clampf(this->param_.share_gain * (iavg - this->sampler_->ibeta_.current_), this->param_.share_limit);
        float igamma_share = abs_clampf(this->param_.share_gain * (iavg - this->sampler_->igamma_.current_), this->param_.share_limit);

        float alpha_cmd =
            volt_feedforward *
            (1 + Component_PID_Calculate(&(this->PID_ialpha_), ibase_setpoint + ialpha_share, this->sampler_->ialpha_.current_, this->dt));

        float beta_cmd = volt_feedforward *
                         (1 + Component_PID_Calculate(&(this->PID_ibeta_), ibase_setpoint + ibeta_share, this->sampler_->ibeta_.current_, this->dt));

        float gamma_cmd =
            volt_feedforward *
            (1 + Component_PID_Calculate(&(this->PID_igamma_), ibase_setpoint + igamma_share, this->sampler_->igamma_.current_, this->dt));

        float avg_cmd = (alpha_cmd + beta_cmd + gamma_cmd) / 3.0f;
        this->buckboost_mode_ = Device_BuckBoost_GetMode(avg_cmd);

        // 5. 电压/电流竞争：高压充电区由电压环对三相电流环输出做下压钳位
        if (this->paside_setpoint_ > 0.0f && this->sampler_->vbside_.voltage_ > this->buckboost_alpha_.CAP_MAX_VOLTAGE * 0.9f)
        {
            float vb_cmd_limit =
                volt_feedforward *
                (1.0f +
                 Component_PID_Calculate(&(this->PID_vbside_), this->buckboost_alpha_.CAP_MAX_VOLTAGE, this->sampler_->vbside_.voltage_, this->dt));

            // 电压环仅作为“下压限制器”参与竞争，不允许反向抬高充电命令
            vb_cmd_limit = CLAMP(vb_cmd_limit, 0.0f, 1.5f);
            vb_cmd_limit = MIN(vb_cmd_limit, volt_feedforward);

            if (vb_cmd_limit < alpha_cmd || vb_cmd_limit < beta_cmd || vb_cmd_limit < gamma_cmd)
            {
                alpha_cmd = MIN(alpha_cmd, vb_cmd_limit);
                beta_cmd = MIN(beta_cmd, vb_cmd_limit);
                gamma_cmd = MIN(gamma_cmd, vb_cmd_limit);

                if (this->base_referee_power_ > this->status_->chassisPowerLimit + 3.0f)
                    this->base_referee_power_ = this->status_->chassisPowerLimit + 3.0f;

                if (this->paside_setpoint_ > 8.0f)
                    this->paside_setpoint_ -= 8.0f;
            }
        }
        else
        {
            Component_PID_Reset(&(this->PID_vbside_));
        }

        this->output_duty_ = (alpha_cmd + beta_cmd + gamma_cmd) / 3.0f;
        this->status_->outputduty = this->output_duty_;

        Device_BuckBoost_UpdatePWM(&(this->buckboost_alpha_), alpha_cmd, this->buckboost_mode_);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_beta_), beta_cmd, this->buckboost_mode_);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_gamma_), gamma_cmd, this->buckboost_mode_);
    }
    else
    {
        // 电压过低，直接停机或进入空闲状态
        float idle_vb_cmd = this->sampler_->vbside_.voltage_ / this->sampler_->vaside_.voltage_;

        this->paside_setpoint_ = 0.0f;
        this->iaside_setpoint_ = 0.0f;
        this->iphase_setpoint_ = 0.0f;

        Component_PID_Reset(&(this->PID_ialpha_));
        Component_PID_Reset(&(this->PID_ibeta_));
        Component_PID_Reset(&(this->PID_igamma_));
        Component_PID_Reset(&(this->PID_vbside_));
        Component_PID_Reset(&(this->PID_pRefree_));

        // 获取全局空闲模式
        this->buckboost_mode_ = Device_BuckBoost_GetMode(idle_vb_cmd);

        Device_BuckBoost_UpdatePWM(&(this->buckboost_alpha_), idle_vb_cmd, this->buckboost_mode_);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_beta_), idle_vb_cmd, this->buckboost_mode_);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_gamma_), idle_vb_cmd, this->buckboost_mode_);
    }
}