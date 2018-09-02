#pragma once

#include "utility.hpp"

#include <stm32f10x.h>

class Pin {
public:
	Pin(GPIO_TypeDef* gpio, int16_t pin, uint16_t mask) : gpio(gpio), pin(pin), mask(mask) {}

	void Set() {
		gpio->BSRR = mask;
	}

	void Clear() {
		gpio->BRR = mask;
	}

	void Toggle() {
		gpio->ODR ^= mask;
	}

	uint16_t Read() {
		return gpio->IDR & mask;
	}

	void Write(uint32_t value) {
		if (value)
			Set();
		else
			Clear();
	}

private:
	GPIO_TypeDef* gpio;
	int16_t pin;
	uint16_t mask;
};

template <unsigned int bits, typename ShiftReg_t = uint32_t>
class FilteredPin {
public:
	enum { mask = ((1 << bits) - 1) };

	FilteredPin(Pin* pin) : pin(pin), shiftReg(0) {}

	bool IsAllSet() volatile { return shiftReg == mask; }
	bool IsAllCleared() volatile { return shiftReg == 0; }

	bool IsAnySet() volatile { return shiftReg != 0; }
	bool IsAnyCleared() volatile { return shiftReg != mask; }

	void Sample() volatile {
		shiftReg = ((shiftReg << 1) | (pin->Read() ? 1 : 0)) & mask;
	}

	ShiftReg_t GetShiftReg() volatile {
		return shiftReg;
	}

private:
	Pin* pin;
	ShiftReg_t shiftReg;
};
