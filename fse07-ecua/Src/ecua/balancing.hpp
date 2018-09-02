/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2015, 2018 eForce FEE Prague Formula
 */

#ifndef ecua_balancing_hpp
#define ecua_balancing_hpp

#include "ecua_options.hpp"

#include <library/timer.hpp>

#include <optional>

enum class BalancingAction_t {
	continue_,
	finish,
	error,
};

// Cell balancing model
// Model assumptions:
// - updated often enough (500msec)
// - ALL data is fresh enough (500msec)
// - reaction time is quick enough (100msec)
class Balancing {
public:
	struct Inputs_t {
		struct StackInfo_t {
			Voltage_t cellVoltages[kBMSCellChannels];
		};

		StackInfo_t stacks[kMaxStacks];
		Voltage_t allowedVoltageDifference;

		// helper vars
		Voltage_t globalMinVoltage;
		Voltage_t maxVoltageDifference;
	};

	struct Outputs_t {
		struct StackInfo_t {
			uint16_t dischargeEnableMask;
		};

		StackInfo_t stacks[kMaxStacks];
		Voltage_t targetCellVoltage;
		//int16_t maxVoltageDifference;
		int timeSinceLastDischarge;
	};

	void Restart();
	BalancingAction_t Update(uint32_t timestamp, const Inputs_t& inputs, Outputs_t& outputs);

private:
	int GetMaxTemp(const Inputs_t& inputs, int stack);
	void StopDischargeInStack(uint32_t timestamp, int stack);

	enum StackBalancingState_t {
		start,
		cooldown,
		discharging,
		finished,
	};

	struct StackInfo_t {
		StackBalancingState_t state;

		uint32_t startTimestamp;
		uint16_t dischargeEnableMask;
		uint16_t coolingDownMask;
		uint32_t coolingDownTimestamps[kBMSCellChannels];
	};

	StackInfo_t stacks[kMaxStacks];
	std::optional<Voltage_t> targetCellVoltage;
	SysTickTimer timeSinceLastDischarge;		// better to have a timestamp in Inputs_t probably
};

#endif
