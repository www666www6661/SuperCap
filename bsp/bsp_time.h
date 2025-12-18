#pragma once

#include <stdint.h>

void bsp_time_init(void);

uint32_t bsp_time_get_ms(void);
uint64_t bsp_time_get_us(void);
uint64_t bsp_time_get(void);