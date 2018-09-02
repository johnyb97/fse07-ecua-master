#pragma once

#include "sdc.hpp"
#include "voltagesense.hpp"

class ECUACAN {
public:
	ECUACAN(Peripherals* peripherals, VoltageSense* voltages, SDC* sdc) : peripherals(peripherals), voltages(voltages), sdc(sdc) {}

	void Init() {}
	void Update();

private:
	Peripherals* peripherals;
	VoltageSense* voltages;
	SDC* sdc;
};
