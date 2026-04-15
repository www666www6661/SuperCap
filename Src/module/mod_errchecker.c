#include "mod_errchecker.h"

#include <math.h>
#include <stdbool.h>

#include "dev_buckboost.h"
#include "dev_buzzer.h"

/**
 * @brief Initializes the error checker module data.
 */
void Module_ErrChecker_Init(Module_ErrChecker *this, Module_ErrChecker_Param param) { this->param_ = param; }

/**
 * @brief Handles short circuit detection and response.
 *        Ported from handleShortCircuit() in PowerManager.cpp.
 */
// void Module_ErrChecker_ShortChk(Module_ErrChecker *this) {
//   if (this->sampler_->vaside_.voltage_ < this->param_.SHORT_CIRCUIT_VOLTAGE) {
//     if (ABS(this->sampler_->iaside_.current_) >
//         this->param_.SHORT_CIRCUIT_CURRENT) {
//       this->status_->errorcode |= ERROR_SHORT_CIRCUIT;
//       if (this->short_circuit_cnt_++ >= 80) {
//         this->short_circuit_cnt_ = 80;
//         this->status_->outputEnabled = false;
//         Device_BuckBoost_Disable();
//         Device_Buzzer_Play(200, 100);
//       }
//     }
//     if (this->sampler_->vbside_.voltage_ < this->param_.SHORT_CIRCUIT_VOLTAGE) {
//       if (ABS(this->sampler_->ibside_.current_) >
//           this->param_.SHORT_CIRCUIT_CURRENT) {
//         if (this->short_circuit_cnt_++ >= 80) {
//           this->short_circuit_cnt_ = 80;
//           this->status_->outputEnabled = false;
//           this->status_->errorcode |= ERROR_SHORT_CIRCUIT;
//         }
//         Device_BuckBoost_Disable();
//         Device_Buzzer_Play(200, 100);
//       }
//     }
//   }
// }
