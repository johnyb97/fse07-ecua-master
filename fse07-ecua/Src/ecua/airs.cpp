/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2015, 2018 eForce FEE Prague Formula
 */

#include "airs.hpp"
#include "bms.hpp"
#include "ecua.hpp"
#include "peripherals.hpp"
#include "sdc.hpp"
#include "voltagesense.hpp"

#include <fse04.h>

#include <library/pin.hpp>
#include <stdlib.h>

void AIRs::Init() {
	debug_printf(("AIRs: Waiting to detect car/charger\r\n"));
	SetState(AIRsState::ready);
}

void AIRs::ErrorFatal(int err) {
	debug_printf(("ErrorFatal(%d)\r\n", err));

	ResetOutputs();
	sdc->Fault();

	if (err == ECUA4_HVM_DISCONNECT || err == ECUA4_HVM_MELT || err == ECUA4_HVM_TIMEOUT ||
			err == ECUA4_HVP_DISCONNECT || err == ECUA4_HVP_MELT || err == ECUA4_HVP_TIMEOUT) {
		airsFault = true;
	}

	SetState(AIRsState::fatalError, err);
}

void AIRs::ErrorNonFatal(int err) {
	debug_printf(("ErrorNonFatal(%d)\r\n", err));

	ResetOutputs();

	if (++errorCounter != kMaxErrors)
		SetState(AIRsState::ready);
	else
		ErrorFatal(ECUA4_TOO_MANY_ERRORS);
}

void AIRs::ErrorWithCooldown(int err) {
	debug_printf(("ErrorWithCooldown(%d)\r\n", err));

	ResetOutputs();

	if (++errorCounter != kMaxErrors) {
		timeout.Restart();
		SetState(AIRsState::cooldown, err);
	}
	else
		ErrorFatal(ECUA4_TOO_MANY_ERRORS);
}

void AIRs::i_OnTick() {
	sdcFiltered.Sample();
	airMFiltered.Sample();
	airPFiltered.Sample();
}

void AIRs::NonErrorWithCooldown(int err) {
	debug_printf(("NonErrorWithCooldown(%d)\r\n", err));

	ResetOutputs();

	timeout.Restart();
	SetState(AIRsState::cooldown, err);
}

void AIRs::ResetOutputs() {
	precharge.Clear();
	airPEn.Clear();
	airMEn.Clear();

	airMClosed = false;
	airPClosed = false;

	voltages->InvalidateVoltages();
}

void AIRs::SetState(AIRsState state, uint8_t extended) {
	//g_flightDiary.Event(kEventStateTransition, (uint8_t)state, extended);

	this->state = state;
	this->errorCode = extended;
}

void AIRs::Update() {
	if (state == AIRsState::fatalError)
		return;

	if (sdcClosed && sdcFiltered.IsAnyCleared()) {
		sdcClosed = false;
		debug_printf(("ERROR: SDC INTERRUPTED\r\n"));

		NonErrorWithCooldown(ECUA4_SDC_INTERRUPTED);
	}

	// FIXME: rozepnuti AIRu za behu nekontrolovat, pri prechargi non-fatal error

/*
	if (airMClosed && airMFiltered.IsAnySet()) {
		debug_printf(("INFO: sdcFilter=%X airMFilter=%X airPFilter=%X\r\n",
				sdcFiltered.GetShiftReg(), airMFiltered.GetShiftReg(), airPFiltered.GetShiftReg()));
		debug_printf(("FATAL ERROR: HV- DISCONNECT\r\n"));
		ErrorFatal(ECUA4_HVM_DISCONNECT);
	}

	if (airPClosed && airPFiltered.IsAnySet()) {
		debug_printf(("INFO: sdcFilter=%X airMFilter=%X airPFilter=%X\r\n",
				sdcFiltered.GetShiftReg(), airMFiltered.GetShiftReg(), airPFiltered.GetShiftReg()));
		debug_printf(("FATAL ERROR: HV+ DISCONNECT\r\n"));
		ErrorFatal(ECUA4_HVP_DISCONNECT);
	}
*/
	switch (state) {
	case AIRsState::pendingChargerDetect:
		if (peripherals->GetEnvironment() != Environment::unknown) {
			debug_printf(("AIRs: Waiting for Shutdown Circuit to close\r\n"));
			SetState(AIRsState::ready);
		}
		break;

	// cekani na uzavreni Shutdown Circuit ridicem
	case AIRsState::ready:
		if (sdcFiltered.IsAllSet()) {
			sdcClosed = true;

			SetState(AIRsState::startupCheck);
			debug_printf(("AIRs: Checking AIRs...\r\n"));
			timeout.Restart();
		}
		break;

	// kontrola AIRu ve vypnutem stavu
	case AIRsState::startupCheck:
		// kvoli stykaci v nabijackach
		if (!timeout.TimeElapsed(1000))
			break;

		if (!airMCon_n.Read()) {
			debug_printf(("FATAL ERROR: HV- MELT\r\n"));
			ErrorFatal(ECUA4_HVM_MELT);
			return;
		}

		if (!airPCon_n.Read()) {
			debug_printf(("FATAL ERROR: HV+ MELT\r\n"));
			ErrorFatal(ECUA4_HVP_MELT);
			return;
		}

		/*if (requireBMS && BMS::IsFault()) {
			debug_printf(("FATAL ERROR: BMS COMM.\r\n"));
			ErrorFatal(ECUA4_BMS_COMMUNICATION);
			return;
		}*/

		if (enableCellVoltageStartCheck && peripherals->GetEnvironment() != Environment::charger) {
			Voltage_t cellMinVoltage;

			if (!BMS::GetMinCellVoltage(&cellMinVoltage)) {
				debug_printf(("FATAL ERROR: BMS COMM.\r\n"));
				ErrorFatal(ECUA4_BMS_COMMUNICATION);
				return;
			}

			if (cellMinVoltage < Voltage_t::fromMillivolts(kCellCutoffVoltage)) {
				printf("FATAL ERROR: cell min voltage %d mV\r\n", cellMinVoltage.toMillivolts());
				//g_flightDiary.Event(kEventCellMinVoltage, cellMinVoltage);
				ErrorFatal(ECUA4_BMS_CELL_MIN_VOLTAGE);
				break;
			}
		}

		if (enableTemperatureStartCheck) {
			auto tempRange = BMS::GetTemperatureRange();

			if (!tempRange.has_value()) {
				debug_printf(("FATAL ERROR: BMS COMM.\r\n"));
				ErrorFatal(ECUA4_BMS_COMMUNICATION);
				return;
			}

			if (peripherals->GetEnvironment() == Environment::charger && *tempRange != TemperatureRange::normal) {
				//printf("FATAL ERROR: cell temp %d degC\r\n", maxTemp_degC);
				ErrorFatal(ECUA4_BMS_CELL_TEMPERATURE);
				break;
			}

			if (peripherals->GetEnvironment() == Environment::car && *tempRange == TemperatureRange::critical) {
				//printf("FATAL ERROR: cell temp %d degC\r\n", maxTemp_degC);
				ErrorFatal(ECUA4_BMS_CELL_TEMPERATURE);
				break;
			}
		}

		/*if (cellImbalance > kMaxCellVoltageDifference) {
			printf("FATAL ERROR: cell imbalance %u mV\r\n", cellImbalance);
			//printf("FATAL WARNING: cell imbalance %u mV\r\n", cellImbalance);
			//g_flightDiary.Event(kEventCellImbalance, cellImbalance);
			ErrorFatal(ECUA4_BMS_CELL_BALANCE);
			break;
		}*/

		debug_printf(("AIRs: Closing HV-\r\n"));
		airMEn.Set();

		// sepnout HV- AIR
		timeout.Restart();
		SetState(AIRsState::hvMClosing);

		break;

	case AIRsState::hvMClosing:
		// HV- sepnul?
		if (airMFiltered.IsAllCleared()) {
			airMClosed = true;
			debug_printf(("AIRs: HV- Closed (took %lu ms)\r\n", timeout.GetTimeElapsed()));

			debug_printf(("AIRs: Connecting Precharge...\r\n"));
			// sepnout precharge a cekat
			precharge.Set();
			timeout.Restart();
			SetState(AIRsState::prechargeClosing);
		}
		// casovy limit pro sepnuti HV- vyprsel
		else if (timeout.TimeElapsed(tAirMCloseTimeout)) {
			debug_printf(("ERROR: HV- TIMEOUT\r\n"));
			ErrorNonFatal(ECUA4_HVM_TIMEOUT);

			if (state != AIRsState::fatalError) {
				SetState(AIRsState::hvmCooldown);
				timeout.Restart();
			}
		}
		break;

	case AIRsState::prechargeClosing:
		// precharge je sepnuty (snad - nemame jak overit, ale to nevadi)
		if (timeout.TimeElapsed(tPrechargeCloseTimeout)) {
			debug_printf(("AIRs: Precharging Motor Controller...\r\n"));

			// zahajit timer pro precharging
			timeout.Restart();
			SetState(AIRsState::precharging);
		}
		break;

	case AIRsState::precharging: {
		// doba precharge zavisi na tom, zda je pripojena nabijecka
		const auto tPrecharge = (peripherals->GetEnvironment() == Environment::charger) ? tPrechargeCharger : tPrechargeAccu;

		if (timeout.TimeElapsed(tPrecharge)) {
			// prejit do stavu cekani na vyrovnani napeti
			debug_printf(("AIRs: Waiting for voltage difference stabilization\r\n"));
			timeout.Restart();
			SetState(AIRsState::waitingForVdiff);
		}
		break;
	}

	case AIRsState::waitingForVdiff: {
		int32_t accu_mV, air_mV;

		//debug_printf(("AIRs: ADC Reading...\r\n"));

		// FIXME: ensure non-negative (elsewhere)
		if (!voltages->GetVoltages(&accu_mV, &air_mV))		// TODO: nejak resit?
			return;

		if (timeout.TimeElapsed(tPrechargeSlopeTimeout) && air_mV < 70000) { // air_mV < accu_mV / 3) {
			debug_printf(("ERROR: PRECHARGE SLOPE\r\n"));
			ErrorWithCooldown(ECUA4_PRECHARGE_SLOPE);
		}

		debug_printf(("AIRs: ACCU %ld mV; AIR %ld mV\r\n", accu_mV, air_mV));

		// napeti srovnane
		if (abs(accu_mV - air_mV) < kVdiffMax * 1000 ) {
			debug_printf(("AIRs: VOLTAGE OK (took %lu ms), Closing HV+\r\n", timeout.GetTimeElapsed()));
			airPEn.Set();

			// sepnout HV+
			timeout.Restart();
			SetState(AIRsState::hvPClosing);
		}
		// napeti se vcas nesrovnalo
		else if (timeout.TimeElapsed(tPrechargeTimeout)) {
			debug_printf(("ERROR: VDIFF TIMEOUT\r\n"));
			ErrorWithCooldown(ECUA4_VDIFF_TIMEOUT);
		}
		break;
	}

	case AIRsState::hvPClosing:
		// HV+ sepnul?
		if (airPFiltered.IsAllCleared()) {
			debug_printf(("AIRs: HV+ Closed (took %lu ms)\r\n", timeout.GetTimeElapsed()));
			airPClosed = true;
			SetState(AIRsState::endPrecharge);
		}
		// casovy limit pro sepnuti HV+ vyprsel
		else if (timeout.TimeElapsed(tAirPCloseTimeout)) {
			debug_printf(("ERROR: HV+ TIMEOUT\r\n"));
			ErrorFatal(ECUA4_HVP_TIMEOUT);
		}
		break;

	case AIRsState::endPrecharge:
		// odpojit precharge a muzeme jet
		debug_printf(("AIRs: Disconnecting Precharge\r\n"));
		precharge.Clear();

		// TODO: configure charger
		//state = AIRsState::configureCharger

		debug_printf(("AIRs: Jedeme!\r\n"));
		SetState(AIRsState::tsActive);

		errorCounter = 0;
		break;

	case AIRsState::tsActive: {
//		if (BMS::isBMSComOk()==false) {
//			debug_printf(("FATAL ERROR: BMS COMM.\r\n"));
//			ErrorFatal(ECUA4_BMS_COMMUNICATION);
//			return;
//		}

		if (enableCellVoltageOperationCheck && peripherals->GetEnvironment() == Environment::car) {
			Voltage_t cellMinVoltage;

			if (!BMS::GetMinCellVoltage(&cellMinVoltage)) {
				debug_printf(("FATAL ERROR: BMS COMM.\r\n"));
				ErrorFatal(ECUA4_BMS_COMMUNICATION);
				return;
			}

			if (cellMinVoltage < Voltage_t::fromMillivolts(kCellCutoffVoltage)) {
				printf("FATAL ERROR: cell min voltage %u mV\r\n", cellMinVoltage.toMillivolts());
				//g_flightDiary.Event(kEventCellMinVoltage, cellMinVoltage);
				ErrorFatal(ECUA4_BMS_CELL_MIN_VOLTAGE);
				break;
			}
		}

		if (enableTemperatureOperationCheck) {
			auto tempRange = BMS::GetTemperatureRange();

			if (!tempRange.has_value()) {
				debug_printf(("FATAL ERROR: BMS COMM.\r\n"));
				ErrorFatal(ECUA4_BMS_COMMUNICATION);
				return;
			}

			if (peripherals->GetEnvironment() == Environment::charger && *tempRange != TemperatureRange::normal) {
				//printf("FATAL ERROR: cell temp %d degC\r\n", maxTemp_degC);
				ErrorFatal(ECUA4_BMS_CELL_TEMPERATURE);
				break;
			}

			if (peripherals->GetEnvironment() == Environment::car && *tempRange == TemperatureRange::critical) {
				//printf("FATAL ERROR: cell temp %d degC\r\n", maxTemp_degC);
				ErrorFatal(ECUA4_BMS_CELL_TEMPERATURE);
				break;
			}
		}
		break;
	}

	case AIRsState::fatalError:
		break;

	//case AIRsState::nonFatalError:
	//	break;

	case AIRsState::cooldown:
		if (timeout.TimeElapsed(tPrechargeCooldown)) {
			debug_printf(("AIRs: Precharge Cooldown End\r\n"));
			SetState(AIRsState::ready);
		}
		break;

	case AIRsState::hvmCooldown:
		if (timeout.TimeElapsed(tAirMCooldown)) {
			debug_printf(("AIRs: HV- Cooldown End -> waiting for SDC\r\n"));
			SetState(AIRsState::ready);
		}
		break;
	}
}
