#pragma once

#include <library/pin.hpp>
#include <library/timer.hpp>

extern "C" {
#include "gpio.h"
}

class SDC {
public:
	SDC(Pin* amsInput, Pin* hvInterlock, Pin* imdOk, Pin* acpOKoutputEN, Pin* SDCend)
			: amsInput(amsInput), hvInterlock(hvInterlock), imdOk(imdOk),
			  acpOK(acpOKoutputEN), SDCend(SDCend) {
			}

	bool Init();
	void Update();

	void Fault();

	static bool IsSdcInPresent() { return 1; }
	static bool IsHvInterlockPresent() { return is_SDC_HVIL_Present; }
	static bool IsImdOk() { return is_IMD_OK; }
	bool IsAmsOk();
	bool IsSdcOutPresent();
	static bool IsSdcEndPresent() { return is_SDC_END_Present; }

	static bool IsLatched() { return is_SDC_Err_Latched; }

private:
	Pin *amsInput, *hvInterlock, *imdOk, *acpOK, *SDCend;
	SysTickTimer imdStartupTimer;

	bool fault = false;

	bool measuringIMD = false;
};
