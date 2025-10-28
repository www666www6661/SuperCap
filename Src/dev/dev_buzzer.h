#pragma once
#include <stdbool.h>

void Buzzer_Start(void);
void Buzzer_Stop(void);
void Buzzer_Set(float freq, float duty_cycle);
void Buzzer_PowerOn(void);