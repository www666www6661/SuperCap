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
        float actual_ia_to_ib = MAX(this->sampler_->vbside_.voltage_, 0.1f) / MAX(this->sampler_->vaside_.voltage_, 0.1f);
        float ibside_est = this->sampler_->iaside_.current_ * this->sampler_->vaside_.voltage_ / MAX(this->sampler_->vbside_.voltage_, 0.1f);

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

        float power_limit_a_to_b = MIN(this->buckboost_alpha_.I_LIMIT * this->sampler_->vaside_.voltage_,
                                       temp_cap_in_ilimit * this->sampler_->vaside_.voltage_ * actual_ia_to_ib);
        float power_limit_b_to_a = MAX(-1 * this->buckboost_alpha_.I_LIMIT * this->sampler_->vaside_.voltage_,
                                       -1 * temp_cap_out_ilimit * this->sampler_->vaside_.voltage_ * actual_ia_to_ib);

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

        // this->iaside_setpoint_ = this->iaside_setpoint_ > 1.0f ? 0.4f : this->iaside_setpoint_ + 0.00005f;
        this->iaside_setpoint_ = 0.8f;

        // =========================================================================

        // 1. 获取物理前馈比率 (V_out / V_in)
        float ff_voltage_ratio = this->sampler_->vbside_.voltage_ / this->sampler_->vaside_.voltage_;

        // 2. 获取当前的系统全局模式 (调用上一轮回复中提供的新函数)
        bool buckboost_mode = Device_BuckBoost_GetBuckBoostMode(ff_voltage_ratio);

        // 3. 计算【单相】目标电流 (总电流除以3)
        float phase_i_setpoint = this->iaside_setpoint_ / 3.0f;

        // 4. 三相电流内环 PID 计算 (前馈 + PID)
        // 这里的 sampler 需要获取三相独立的电流，名称请根据你实际的结构体修改
        float duty_alpha = ff_voltage_ratio + this->param_.k_feedforward * phase_i_setpoint +
                           Component_PID_Calculate(&(this->PID_ialpha_), phase_i_setpoint, this->sampler_->i_alpha_.current_, this->dt);

        float duty_beta = ff_voltage_ratio + this->param_.k_feedforward * phase_i_setpoint +
                          Component_PID_Calculate(&(this->PID_ibeta_), phase_i_setpoint, this->sampler_->i_beta_.current_, this->dt);

        float duty_gamma = ff_voltage_ratio + this->param_.k_feedforward * phase_i_setpoint +
                           Component_PID_Calculate(&(this->PID_igamma_), phase_i_setpoint, this->sampler_->i_gamma_.current_, this->dt);

        // 5. 电压限幅控制
        if (this->sampler_->vbside_.voltage_ > this->buckboost_alpha_.CAP_MAX_VOLTAGE * 0.9f)
        {
            float vb_duty =
                Component_PID_Calculate(&(this->PID_vbside_), this->buckboost_alpha_.CAP_MAX_VOLTAGE, this->sampler_->vbside_.voltage_, this->dt) +
                ff_voltage_ratio;

            // 如果电压环要求减小占空比，则强制钳位三相的输出
            if (vb_duty < duty_alpha || vb_duty < duty_beta || vb_duty < duty_gamma)
            {  // 只要有一个超出，就取最小限制
                duty_alpha = MIN(duty_alpha, vb_duty);
                duty_beta = MIN(duty_beta, vb_duty);
                duty_gamma = MIN(duty_gamma, vb_duty);

                // 保留原有的越限逻辑标志位更新
                this->output_duty_ = vb_duty;
                if (this->base_referee_power_ > this->status_->chassisPowerLimit + 3.0f)
                    this->base_referee_power_ = this->status_->chassisPowerLimit + 3.0f;
                if (this->paside_setpoint_ > ibside_est * this->buckboost_alpha_.CAP_MAX_VOLTAGE + 8.0f)
                    this->paside_setpoint_ = ibside_est * this->buckboost_alpha_.CAP_MAX_VOLTAGE + 8.0f;
            }
            else
            {
                this->output_duty_ = (duty_alpha + duty_beta + duty_gamma) / 3.0f;  // 仅用于日志记录
            }
        }
        else
        {
            Component_PID_Reset(&(this->PID_vbside_));
            this->output_duty_ = (duty_alpha + duty_beta + duty_gamma) / 3.0f;  // 仅用于日志记录
        }
        this->status_->outputduty = this->output_duty_;

        // 钳位三相输出
        clampf(&duty_alpha, 0.05f, 10.0f);
        clampf(&duty_beta, 0.05f, 10.0f);
        clampf(&duty_gamma, 0.05f, 10.0f);

        // 6. 更新三相底层 PWM (注意这里使用相同的 sys_mode)
        Device_BuckBoost_UpdatePWM(&(this->buckboost_alpha_), duty_alpha, buckboost_mode);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_beta_), duty_beta, buckboost_mode);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_gamma_), duty_gamma, buckboost_mode);
    }
    else
    {
        // 电压过低，直接停机或进入空闲状态
        float idle_duty = this->sampler_->vbside_.voltage_ / this->sampler_->vaside_.voltage_;
        clampf(&idle_duty, 0.05f, 10.0f);

        this->paside_setpoint_ = 0.0f;
        this->iaside_setpoint_ = 0.0f;

        Component_PID_Reset(&(this->PID_ialpha_));
        Component_PID_Reset(&(this->PID_ibeta_));
        Component_PID_Reset(&(this->PID_igamma_));
        Component_PID_Reset(&(this->PID_vbside_));
        Component_PID_Reset(&(this->PID_pRefree_));

        // 获取全局空闲模式
        bool buckboost_mode = Device_BuckBoost_GetBuckBoostMode(idle_duty);

        Device_BuckBoost_UpdatePWM(&(this->buckboost_alpha_), idle_duty, buckboost_mode);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_beta_), idle_duty, buckboost_mode);
        Device_BuckBoost_UpdatePWM(&(this->buckboost_gamma_), idle_duty, buckboost_mode);
    }
}