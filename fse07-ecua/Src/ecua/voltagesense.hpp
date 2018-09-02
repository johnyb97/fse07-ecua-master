#pragma once

#include "windowcomparator.hpp"

#include <stdint.h>

#include <library/timer.hpp>

class VoltageSense {
public:
	void Init();
	void Update();

	int GetSOCPercent();
	bool GetVoltages(int32_t* accu_mV_out, int32_t* airs_mV_out);
	void InvalidateVoltages() { accuVoltageValid = airsVoltageValid = false; }

private:
	SysTickTimer updateTimer;

	bool accuVoltageValid = false, airsVoltageValid = false;
	int32_t accu_mV, airs_mV, capGND_mV;

	int counter1Sec;
	WindowComparator<int, 10> comparator;
};
