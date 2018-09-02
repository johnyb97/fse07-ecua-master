
#include "airs.hpp"
#include "bms.hpp"
#include "ecuacan.hpp"
#include "ecua_options.hpp"
#include "peripherals.hpp"
#include "sdc.hpp"

#include <can_ECUA.h>

// BMS Tx
static int AMSTemp_StackID;
static int AMSTemp_SetID;
static int AMSVolts_StackID;
static int AMSVolts_SetID;

void ECUACAN::Update() {
	if (ECUA_Status_need_to_send()) {
		auto tempRange = BMS::GetTemperatureRange();

		ECUA_Status_t status;

		status.SDC_IN_Open =	!SDC::IsSdcInPresent();
		status.SDC_HV_ILOCK =	SDC::IsHvInterlockPresent();
		status.SDC_IMD =		SDC::IsImdOk();
		status.SDC_AMS =		sdc->IsAmsOk();
		status.SDC_OUT =		sdc->IsSdcOutPresent();
		status.SDC_END =		SDC::IsSdcEndPresent();
		status.LATCH_SDC_AMS =	SDC::IsLatched();

		status.AIRsState =		(ECUA_AIRsState)g_airs.GetState();
		status.AIRs_errno =		g_airs.GetErrorCode();

		status.FT_ACP_OT =		(!tempRange.has_value() || tempRange == TemperatureRange::critical);
		status.FT_AIRS =		g_airs.IsAIRsFault() ? 1 : 0;
		status.FT_DCDC =		0;				// no way to detect
		status.FT_FAN1 =		0;				// no way to detect
		status.FT_FAN2 =		0;				// no way to detect
		status.FT_FAN3 =		0;				// no way to detect

		int32_t accu_mV = -1, airs_mV = -1;
		if (voltages->GetVoltages(&accu_mV, &airs_mV)) {
			status.FT_HV_OV = (accu_mV > kAcpOVThreshold_mV) ? 1 : 0;
			status.FT_HV_UV = (accu_mV < kAcpUVThreshold_mV) ? 1 : 0;
		}
		else {
			status.FT_HV_OV = 1;
			status.FT_HV_UV = 1;
		}

		status.FT_GLV_UV =		0;				// no way to detect
		status.FT_GLV_OV =		0;				// no way to detect
		status.FT_AMS =			!sdc->IsAmsOk();
		status.FT_ANY =			(status.FT_ACP_OT || status.FT_AIRS || status.FT_DCDC ||
										status.FT_FAN1 || status.FT_FAN2 || status.FT_FAN3 ||
										status.FT_HV_OV || status.FT_HV_UV || status.FT_GLV_UV || status.FT_GLV_OV ||
										status.FT_AMS);
		status.WARN_TEMP_Cell = (!tempRange.has_value() || tempRange != TemperatureRange::normal);
		status.WARN_TEMP_DCDC = 0;				// no way to detect
		status.WARN_TEMP_Bal =	0;				// no way to detect

		status.DCDC_GLV_EN =	peripherals->IsDcdcGlvActive();
		status.FANS_EN =		peripherals->AreFansActive();

		static int seq;
		status.SEQ = 0x0F & ++seq;
		ECUA_send_Status_s(&status);
	}

	if (ECUA_ACPMeas_need_to_send()) {
		int32_t accu_mV = -1, airs_mV = -1;

		if (voltages->GetVoltages(&accu_mV, &airs_mV)) {
			ECUA_ACPMeas_t meas;
			meas.Volt_HV_in = accu_mV / 10;
			meas.Volt_HV_out = airs_mV / 10;
			meas.Curr_HV_out = 0;
			meas.Curr_DCDC_out = 0;
			meas.Curr_FANS = 0;
			ECUA_send_ACPMeas_s(&meas);
		}
	}

	if (CCU_ChargerHeartbeat_need_to_send()) {
		CCU_send_ChargerHeartbeat(0xAA, g_airs.IsTsFullyActive() ? 1 : 0);
	}

	if (ECUA_Limits_need_to_send()) {
		int powerLimit_W = 100000;

		//...

		if (powerLimit_W < 10000) {
			powerLimit_W = 10000;
		}

		ECUA_Limits_t limits;
		limits.PWR_OUT = powerLimit_W / 5;
		limits.PWR_IN = 0;
		ECUA_send_Limits_s(&limits);
	}

	if (ECUA_Estimation_need_to_send()) {
		ECUA_Estimation_t estimation;
		estimation.Charge_IN = 0;
		estimation.Charge_OUT = 0;
		estimation.SOC = voltages->GetSOCPercent() * 2;
		ECUA_send_Estimation_s(&estimation);
	}

	if (ECUA_AMSOverall_need_to_send()) {
		Voltage_t voltMin, voltMax;
		int8_t maxTemp_degC;

		if (BMS::GetSummary(&voltMin, &voltMax, &maxTemp_degC)) {
			ECUA_AMSOverall_t overall;
			overall.Volt_Min = voltMin.toTenthsMillivolt();
			overall.Volt_Max = voltMax.toTenthsMillivolt();
			overall.Temp_Min = 0;
			overall.Temp_Max = maxTemp_degC;
			overall.Volt_Min_index = -1;
			overall.Temp_Max_Index = -1;
			ECUA_send_AMSOverall_s(&overall);
		}
	}

	int num_stacks = BMS::GetNumStacks();

	if (AMSTemp_StackID >= num_stacks) {
		AMSTemp_StackID = 0;
	}

	if (AMSVolts_StackID >= num_stacks) {
		AMSVolts_StackID = 0;
	}

	while (ECUA_AMSTemp_need_to_send()) {
		ECUA_AMSTemp_t temps;
		temps.StackID = AMSTemp_StackID;
		temps.SetID = AMSTemp_SetID;

		bool fail = false;

		for (int i = 0; i < 6; i++) {
			// This may fail for a variety of reasons, e.g. BMS disconnected, measurement not ready yet
			if (!BMS::GetTempAt(temps.StackID, temps.SetID * 6 + i, &temps.Temp[i])) {
				fail = true;
				break;
			}
		}

		if (fail || ECUA_send_AMSTemp_s(&temps) < 0) {
			break;
		}

		if (++AMSTemp_SetID == kBMSTempsPerStack / 6) {
			AMSTemp_SetID = 0;

			if (++AMSTemp_StackID == num_stacks) {
				BMS::FinishFrameTemps();
				AMSTemp_StackID = 0;
			}
		}

		break;
	}

	while (ECUA_AMSVolts_need_to_send()) {
		ECUA_AMSVolts_t volts;
		volts.StackID = AMSVolts_StackID;
		volts.SetID = AMSVolts_SetID;

		Voltage_t voltages[2];
		int timestamp;

		if (!BMS::GetVoltageAt(volts.StackID, volts.SetID * 2, &voltages[0], &timestamp)
				|| !BMS::GetVoltageAt(volts.StackID, volts.SetID * 2 + 1, &voltages[1], &timestamp)) {
			break;
		}

		volts.Volt[0] = voltages[0].toTenthsMillivolt();
		volts.Volt[1] = voltages[1].toTenthsMillivolt();
		volts.Timestamp = timestamp;

		if (ECUA_send_AMSVolts_s(&volts) < 0) {
			break;
		}

		if (++AMSVolts_SetID == kBMSCellChannels / 2) {
			AMSVolts_SetID = 0;

			if (++AMSVolts_StackID == num_stacks) {
				BMS::FinishFrameCells();
				AMSVolts_StackID = 0;
			}
		}

		;
		break;
	}

	if (peripherals->GetEnvironment() == Environment::unknown) {
		if (CCU_get_Announce()) {
			peripherals->SetEnvironment(Environment::charger);
		}
		else if (ECUB_get_Status(nullptr)) {
			peripherals->SetEnvironment(Environment::car);
		}
	}
}

extern "C" {
#include "can.h"
}

#include <tx2/can.h>

extern "C" uint32_t txGetTimeMillis() {
    return HAL_GetTick();
}

extern "C" int txHandleCANMessage(uint32_t timestamp, int bus, CAN_ID_t id, const void* data, size_t length) {
    return 0;
}

extern "C" int txSendCANMessage(int bus, CAN_ID_t id, const void* data, size_t length) {
	if (CAN1_TransmitMessage(id, (const uint8_t*)data, length)) {
		return 0;
	}
	else {
		return -TX_SEND_BUFFER_OVERFLOW;
	}
}
