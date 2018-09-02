#pragma once

#include <stdint.h>

extern volatile uint32_t SystemTicks;

extern "C" void Delay_ms(uint32_t ms);
#define HAL_Delay Delay_ms

inline uint32_t HAL_GetTick() {
	return SystemTicks;
}
