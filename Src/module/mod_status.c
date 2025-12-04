#include "mod_status.h"

void Module_Status_Init(Module_Status *this) {
  this->outputEnabled = false;
  this->errorCode = 0;
  this->capEnergy = 0.0f;
  this->chassisPower = 0.0f;
  this->chassisPowerLimit = 0.0f;
  this->lowBattery = false;
  this->realVBToVA = 0.0f;
  // this->communicationTimeoutCnt = 0;
}
