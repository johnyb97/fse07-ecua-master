/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#include "airs.hpp"
#include "bms.hpp"
#include "ecua_options.hpp"
#include "peripherals.hpp"

extern "C" {
#include "gpio.h"
#include "timer.h"
}

static int fans_RPM = 0;
static SysTickTimer fansUpdateTimer;

template <typename T> T min(T a, T b) { return a < b ? a : b; }
template <typename T> T max(T a, T b) { return a > b ? a : b; }

void Peripherals::Init() {
	fansUpdateTimer.Restart();

	fans_RPM = 0;
	SetFANSpeed(0);
}

void Peripherals::SetEnvironment(Environment env) {
	if (this->environment == Environment::unknown) {
		this->environment = env;
	}
}

void Peripherals::Update() {
	// DCDC -- Start after soft-start time

	if (hvInVoltsSampleTimer.TimeElapsed(200)) {
		int32_t accu_mV, airs_mV;
		if (voltages->GetVoltages(&accu_mV, &airs_mV)) {
			hvInVolts.PushValue(accu_mV / 1000);
		}
		else {
			hvInVolts.PushValue(0);
		}

		hvInVoltsSampleTimer.Restart();
	}

	switch (dcdcState) {
	case DcdcState::off: {
		const bool enableDCDC = (enableHVDCRelay && (kDcdcEnableDuringCharging || GetEnvironment() == Environment::car));

		if (enableDCDC && hvInVolts.AreAllValuesAbove(kDcdcMinInputVolts)) {
			DCDC_HVRELAY_On();
			dcdcState = DcdcState::precharging;
			dcdcPrechargeTimer.Restart();
		}
		break;
	}

	case DcdcState::precharging:
		if (!hvInVolts.AreAllValuesAbove(kDcdcMinInputVolts)) {
			dcdcState = DcdcState::disable;
			break;
		}

		if (dcdcPrechargeTimer.TimeElapsed(tDcdcSoftStart)) {
			if (enableHVDCGlv) {
				// enabled only GLV or both GLV & FANS -- start GLV and go to state "GLV started"
				DCDC_GLV_Enable();
				glvActive = true;
			}

			dcdcState = DcdcState::started1;
			dcdcPrechargeTimer.Restart();
		}
		break;

	case DcdcState::started1:
		if (!hvInVolts.AreAllValuesAbove(kDcdcMinInputVolts)) {
			dcdcState = DcdcState::disable;
			break;
		}

		if (dcdcPrechargeTimer.TimeElapsed(tDcdcEnableInterval)) {
			if (enableHVDCFans) {
				// enabled GAIA & FANS -- start fans and go to state "done"
				DCDC_FAN_Enable();
				fansActive = true;
			}

			dcdcState = DcdcState::startedAll;
			dcdcPrechargeTimer.Restart();
		}
		break;

	case DcdcState::startedAll:
		if (!hvInVolts.AreAllValuesAbove(kDcdcMinInputVolts)) {
			dcdcState = DcdcState::disable;
			break;
		}
		break;

	case DcdcState::disable:
		DCDC_FAN_Disable();
		DCDC_GLV_Disable();
		fansActive = false;
		glvActive = false;
		dcdcState = DcdcState::disabling;
		dcdcPrechargeTimer.Restart();
		break;

	case DcdcState::disabling:
		if (dcdcPrechargeTimer.TimeElapsed(100)) {
			DCDC_HVRELAY_Off();
			dcdcState = DcdcState::off;
		}
		break;
	}

	// FANS -- Calculate target RPM
	// Logic:
	//	- determine RPM by cell temperature (ACTIVE->HIGH ramp)
	//  - determine RPM by operating mode (250 for LV ON, 500 TS ON)
	//  - take the highest result of the above

	if (fansActive && fansUpdateTimer.TimeElapsed(tFansUpdateInterval)) {
		fansUpdateTimer.Restart();

		int base_speed;

		if (g_airs.IsTsFullyActive()) {
			base_speed = kFansActiveSpeed;
		}
		else {
			base_speed = kFansIdleSpeed;
		}

		int tempMax_degC;
		int cooling_speed;
		if (BMS::GetMaxTemp(&tempMax_degC)) {
			if (tempMax_degC < kFansActiveTemp_degC) {
				cooling_speed = 0;
			}
			else if (tempMax_degC < kFansMaxTemp_degC) {
				cooling_speed = kFansActiveSpeed + (kFansMaxSpeed - kFansActiveSpeed) *
						(tempMax_degC - kFansActiveTemp_degC) / (kFansMaxTemp_degC - kFansActiveTemp_degC);
			}
			else {
				cooling_speed = kFansMaxSpeed;
			}
		}
		else {
			cooling_speed = kFansActiveSpeed;
		}

		int target_speed = max(base_speed, cooling_speed);
		target_speed = min(target_speed, int(kFansMaxSpeed));

		// Update current RPM setting

		if (fans_RPM < target_speed) {
			fans_RPM = min(fans_RPM + kFansRampSlope, target_speed);
		}
		else if (fans_RPM > target_speed) {
			fans_RPM = max(fans_RPM - kFansRampSlope, target_speed);
		}

		SetFANSpeed(fans_RPM);
	}
}
