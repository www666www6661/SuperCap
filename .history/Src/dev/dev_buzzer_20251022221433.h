#pragma once
#include <stdbool.h>

bool Buzzer_Start(void);
bool Buzzer_Stop(void);
bool Buzzer_Set(float freq, float duty_cycle);