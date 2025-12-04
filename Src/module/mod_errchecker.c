#include "mod_errchecker.h"
#include "dev_buckboost.h"
#include "dev_buzzer.h"

#include <math.h>
#include <stdbool.h>

#define SHORT_CIRCUIT_VOLTAGE 1.0f
#define SHORT_CIRCUIT_CURRENT 5.0f

// Global instance of the error checker data
Module_ErrChecker_Data module_errchecker_data;

/**
 * @brief Initializes the error checker module data.
 */
void Module_ErrChecker_Init() {
  module_errchecker_data.currentError = 0;
  module_errchecker_data.shortCircuitCnt = 0;
  module_errchecker_data.restartCoolDown = 0;
  module_errchecker_data.errorCoolDown = 0;
}

/**
 * @brief Handles short circuit detection and response.
 *        Ported from handleShortCircuit() in PowerManager.cpp.
 */
void Module_ErrChecker_ShortCircuit() {
#ifndef CALIBRATION_MODE
  if (Module_SampleManager_.vaside_.voltage_ < SHORT_CIRCUIT_VOLTAGE) {
    if (fabsf(Module_SampleManager_.iaside_.current_) > SHORT_CIRCUIT_CURRENT) {
      module_errchecker_data.currentError |= ERROR_SHORT_CIRCUIT;
      // Status::status.errorCode |= ERROR_SHORT_CIRCUIT; // TODO: Adapt or
      // remove global status logic
      Device_BuckBoost_Enable();
      if (module_errchecker_data.shortCircuitCnt++ >= 5) {
        module_errchecker_data.shortCircuitCnt = 5;
        // ControlData::controlData.enableOutput = false; // TODO: Adapt or
        // remove global control logic
      }
      Device_BuckBoost_Disable();
      module_errchecker_data.restartCoolDown = 500;
      module_errchecker_data.errorCoolDown = 500;
      Device_Buzzer_Play(200, 100);
      module_errchecker_data.restartCoolDown = 200;
    }
  }
  if (Module_SampleManager_.vbside_.voltage_ < SHORT_CIRCUIT_VOLTAGE) {
    if (fabsf(Module_SampleManager_.ibside_.current_) > SHORT_CIRCUIT_CURRENT) {
      module_errchecker_data.currentError |= ERROR_SHORT_CIRCUIT;
      // Status::status.errorCode |= ERROR_SHORT_CIRCUIT; // TODO: Adapt or
      // remove global status logic
      Device_BuckBoost_Enable();
      if (module_errchecker_data.shortCircuitCnt++ > 5) {
        module_errchecker_data.shortCircuitCnt = 5;
        // ControlData::controlData.enableOutput = false; // TODO: Adapt or
        // remove global control logic
      }
      module_errchecker_data.restartCoolDown = 500;
      module_errchecker_data.errorCoolDown = 500;
      Device_BuckBoost_Disable();
      Device_Buzzer_Play(200, 100);
      module_errchecker_data.restartCoolDown = 200;
    }
  }
#endif
}
