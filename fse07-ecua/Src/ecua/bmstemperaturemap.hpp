/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#ifndef ecua_bmstemperaturemap_hpp
#define ecua_bmstemperaturemap_hpp

#include "ecua_options.hpp"

class BmsTemperatureMap {
public:
	enum Quality {
		good = 0,
		suspicious = 1,
		bad = 2,
		unknown = 3,
	};

	void Clear(Quality quality);
	Quality Get(int stack, int index);
	void Put(int stack, int index, Quality quality);
	int GetPercentageAcceptable();

	bool LoadFromFlash(uint32_t address);
	void SaveToFlash(uint32_t address);

private:
	static constexpr size_t rows = (kBMSTempsPerStack + 3) / 4;

	// 102 bytes for 6x66
	uint8_t data[kMaxStacks][rows];
};

#endif
