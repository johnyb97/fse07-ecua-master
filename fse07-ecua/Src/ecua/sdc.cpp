#include "bms.hpp"
#include "ecua_options.hpp"
#include "sdc.hpp"

#include "gpio.h"

bool SDC::Init() {
	imdStartupTimer.Restart();

	return true;
}

void SDC::Fault() {
	fault = true;
	AMS_Set_Fail();
}

bool SDC::IsAmsOk() {
	return !fault;
}

bool SDC::IsSdcOutPresent() {
	return this->IsAmsOk() && !IsLatched() && IsHvInterlockPresent() && IsImdOk();
}

void SDC::Update() {
	if (!measuringIMD && imdStartupTimer.TimeElapsed(tIMDStartup)) {
		measuringIMD = true;
	}
}
