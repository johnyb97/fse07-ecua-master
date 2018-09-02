#include "ecua_options.hpp"
#include "voltagesense.hpp"

extern "C" {
#include "mcp3302.h"
}

bool haveInitialMeasurement = false;

void VoltageSense::Init() {
	updateTimer.Restart();
}

int VoltageSense::GetSOCPercent() {
	if (accu_mV < kAcpCutoffVoltage_mV) {
		return 0;
	}
	else if (accu_mV < kAcpFullVoltage_mV) {
		return (accu_mV - kAcpCutoffVoltage_mV) / ((kAcpFullVoltage_mV - kAcpCutoffVoltage_mV) / 100);
	}
	else {
		return 100;
	}
}

bool VoltageSense::GetVoltages(int32_t* accu_mV_out, int32_t* airs_mV_out) {
	if (!accuVoltageValid || !airsVoltageValid)
		return false;

	*accu_mV_out = accu_mV;
	*airs_mV_out = airs_mV;
	return true;
}

void VoltageSense::Update() {
	if (!haveInitialMeasurement || updateTimer.TimeElapsed(tVoltageSensePeriod)) {
		updateTimer.Restart();

		const static int accu_measured[2] = {0, 357500};
		const static int accu_true[2] = {0, 354000};

		const static int airs_measured[2] = {0, 300000};
		const static int airs_true[2] = {0, 300000};

		accu_mV = int(101 * Measure_HV_In() * (float(accu_true[1]) / float(accu_measured[1])));
		airs_mV = int(101 * Measure_HV_Out() * (float(airs_true[1]) / float(airs_measured[1])));

		accuVoltageValid = true;
		airsVoltageValid = true;

		if (++counter1Sec == 1000 / tVoltageSensePeriod) {
			comparator.PushValue(accu_mV);
			counter1Sec = 0;
		}

		haveInitialMeasurement = true;
	}
}
