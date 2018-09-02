/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#ifndef ecua_bms_hpp
#define ecua_bms_hpp

#include "types.hpp"

#include <optional>

class Peripherals;
class SDC;
struct PackCellMeasurementFrame;

enum class TemperatureRange {
	normal,
	warning,
	critical,
};

class BMS {
public:
	// High-level state machine
	// Expected flow:
	//  - init
	//  - pendingSelfTest
	//  - selfTest


	enum class HighState {
		fault,
		pendingInit,
		pendingSelfTest,
		selfTest,
		ready,
		measuring,

		communicationReset,
		//suspend,
	};

	// Low-level state machine

	enum class LowState {
		ready,
		settingMux,
		auxSampling,
		auxReadout,
		auxMeasurementFinished,
		cellReadout,
		cellMeasurementFinished,
		//suspend,
	};

	// Fault reason

	enum class FaultReason {
		baseCommError,
		commError,
		chainLengthMismatch,
		deviceDisconnect,
	};

	static void Init(Peripherals* peripherals, SDC* sdc);
	static void Update();
	//static void Suspend();
	static void Restart();

	static bool IsFault();

	static int GetNumStacks();
	static bool GetSummary(Voltage_t* voltMin_out, Voltage_t* voltMax_out, int8_t* maxTemp_degC_out);
	static bool GetMaxTemp(int* maxTemp_degC);
	static std::optional<TemperatureRange> GetTemperatureRange();
	static bool GetVoltageAt(int stack, int cell, Voltage_t* voltage_out, int* timestamp_out);

	// Temp in half-deg C with 0 == -25C
	// Note: this api is fucking shit
	static bool GetTempAt(int stack, int index, uint8_t* temp_out);

	static bool GetMinCellVoltage(Voltage_t* voltMin_out);

	static void FinishFrameCells();
	static void FinishFrameTemps();

	static void i_UartReceiveData(uint8_t data);

private:
	static void p_ResetCommunication();
	static void p_SetFault(FaultReason reason);
	static void p_SetLowState(LowState state);
	static void p_SetHighState(HighState state);
	static void p_SetStackIndicationLEDs(int stacks);
	static void p_StartAuxMeasurement(int mux);
	static void p_StartCellMeasurement();
	static void p_UpdateBalancingModel(PackCellMeasurementFrame* frame);
	static void p_UpdateHigh();
	static void p_UpdateLow();
};

#endif
