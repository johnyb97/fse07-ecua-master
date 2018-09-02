#pragma once

#include "ecua_options.hpp"

#include <library/pin.hpp>
#include <library/timer.hpp>

enum class AIRsState {
	pendingChargerDetect = 0,
	ready = 1,
	startupCheck = 2,
	prechargeClosing = 3,
	hvMClosing = 4,
	precharging = 5,
	waitingForVdiff = 6,
	hvPClosing = 7,
	endPrecharge = 8,
	//configureCharger,

	tsActive = 9,

	fatalError = 10,
	//nonFatalError = 11,
	cooldown = 12,
	hvmCooldown = 13,
};

class Peripherals;
class SDC;
class VoltageSense;

class AIRs {
public:
	AIRs(Peripherals* peripherals, SDC* sdc, VoltageSense* voltages, Pin* SDCend, Pin* airMCon_n, Pin* airPCon_n) :
		peripherals(peripherals), sdc(sdc), voltages(voltages), sdcFiltered(SDCend), airMFiltered(airMCon_n), airPFiltered(airPCon_n) {
	}

	void Init();
	void Update();
	void Shutdown() { ResetOutputs(); }

	AIRsState GetState() { return state; }
	int GetErrorCode() { return errorCode; }

	// Per FSE rules, the TS is active as soon as precharge starts. We use the term "fully active" to distinguish.
	bool IsTsFullyActive() { return (state == AIRsState::tsActive); }
	bool IsAIRsFault() { return airsFault; }
	bool IsFatalError() { return (state == AIRsState::fatalError); }

	void i_OnTick();

	void ErrorFatal(int err);

protected:
	void ResetOutputs();

	void ErrorNonFatal(int err);
	void ErrorWithCooldown(int err);
	void NonErrorWithCooldown(int err);
	void SetState(AIRsState state, uint8_t extended = 0);

private:
	AIRsState state = AIRsState::pendingChargerDetect;
	int errorCode = -1;
	int errorCounter = 0;

	Peripherals* peripherals;
	SDC* sdc;
	VoltageSense* voltages;
	SysTickTimer timeout;

	volatile FilteredPin<8, uint8_t> sdcFiltered, airMFiltered, airPFiltered;
	bool sdcClosed = false, airMClosed = false, airPClosed = false;

	bool airsFault = false;
};

extern AIRs g_airs;
