/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#ifndef ecua_types_hpp
#define ecua_types_hpp

#include <stdint.h>

struct Voltage_t {
	constexpr static Voltage_t fromMillivolts(unsigned int millivolts) { return Voltage_t { (uint16_t)(millivolts * 65535 / 5000) }; }
	constexpr static Voltage_t fromRaw(uint16_t raw) { return Voltage_t { raw }; }

	bool operator < (Voltage_t other) const { return this->raw < other.raw; }
	bool operator > (Voltage_t other) const { return this->raw > other.raw; }
	Voltage_t operator - (Voltage_t other) const { return Voltage_t { (uint16_t)(this->raw - other.raw) }; }

	int toMillivolts() { return raw * 5000 / 65535; }
	int toTenthsMillivolt() { return raw * 50000 / 65535; }

	float toVolts() { return raw * (5.0f / 65535.0f); }

	uint16_t raw;
};

#endif
