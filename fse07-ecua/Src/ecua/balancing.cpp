/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2015, 2018 eForce FEE Prague Formula
 */

#include "balancing.hpp"

enum {
	// milliseconds
	tDischargeCycle = 15000,
	tVoltageStabilization = 5000,

	kMaxActiveDischargingCells = 2,
};

template <typename T>
T max(const T* array, size_t length) {
	T max_ = array[0];

	for (size_t i = 1; i < length; i++) {
		if (array[i] > max_)
			max_ = array[i];
	}

	return max_;
}

void Balancing::Restart() {
	for (int stack = 0; stack < kMaxStacks; stack++) {
		auto& s = stacks[stack];

		s.state = StackBalancingState_t::start;
		s.dischargeEnableMask = 0;
		s.startTimestamp = 0;
		s.coolingDownMask = 0;
		//s.coolingDownTimestamps = ...;
	}

	targetCellVoltage.reset();
}

BalancingAction_t Balancing::Update(uint32_t timestamp, const Balancing::Inputs_t& inputs, Balancing::Outputs_t& outputs) {
	int numFinishedStacks = 0;

	//printf("Balancing::Update\n");

	if (!targetCellVoltage.has_value() || inputs.globalMinVoltage > *targetCellVoltage || timeSinceLastDischarge.TimeElapsed(60*1000)) {
		targetCellVoltage = inputs.globalMinVoltage;
		//printf("Balancing target set to %d mV, Vdiff=%d\r\n", targetCellVoltage, inputs.maxVoltageDifference);

		if (*targetCellVoltage < kBalancingMinTargetVoltage)
			targetCellVoltage = kBalancingMinTargetVoltage;
	}

	outputs.targetCellVoltage = *targetCellVoltage;

	for (int stack = 0; stack < kMaxStacks; stack++) {
		auto& s = stacks[stack];

		// conservative default
		outputs.stacks[stack].dischargeEnableMask = 0;

		switch (s.state) {
		case StackBalancingState_t::start:
			s.state = StackBalancingState_t::cooldown;
			s.startTimestamp = timestamp;
			break;

		case StackBalancingState_t::cooldown: {
			//auto stackMaxTemp = GetMaxTemp(inputs, stack);

			// decide what to do next
			s.dischargeEnableMask = 0;
			unsigned int numCellsToDischarge = 0;
			unsigned int numCellsChecked = 0;

			int highest_index = 0;
			Voltage_t highest = inputs.stacks[stack].cellVoltages[highest_index];

			for (int i = 1; i < kBMSCellChannels; i++) {
				if (inputs.stacks[stack].cellVoltages[i] > highest) {
					highest = inputs.stacks[stack].cellVoltages[i];
					highest_index = i;
				}
			}

			int highest2_index = (highest_index == 0) ? 1 : 0;
			Voltage_t highest2 = inputs.stacks[stack].cellVoltages[highest2_index];

			for (int i = 0; i < kBMSCellChannels; i++) {
				if (i != highest_index && inputs.stacks[stack].cellVoltages[i] > highest2) {
					highest2 = inputs.stacks[stack].cellVoltages[i];
					highest2_index = i;
				}
			}

			if (highest > *targetCellVoltage && highest - *targetCellVoltage > inputs.allowedVoltageDifference) {
				s.dischargeEnableMask |= (1 << highest_index);
				numCellsToDischarge++;
			}

			if (highest2 > *targetCellVoltage && highest2 - *targetCellVoltage > inputs.allowedVoltageDifference) {
				s.dischargeEnableMask |= (1 << highest2_index);
				numCellsToDischarge++;
			}

			numCellsChecked = kBMSCellChannels;

			// all cells have been sufficiently discharged - this stack is done
			if (numCellsToDischarge == 0 && numCellsChecked == kBMSCellChannels) {
				printf("[%d] stack balancing finished\r\n", stack);
				s.state = StackBalancingState_t::finished;
				break;
			}

			if (numCellsToDischarge == 0)
				break;

			printf("[%d] starting discharge with mask %04X\r\n", stack, s.dischargeEnableMask);
			s.state = StackBalancingState_t::discharging;
			s.startTimestamp = timestamp;
			outputs.stacks[stack].dischargeEnableMask = s.dischargeEnableMask;
			break;
		}

		case StackBalancingState_t::discharging: {
			// check cycle time
			if (timestamp >= s.startTimestamp + tDischargeCycle) {
				printf("[%d] stopping discharge after %lu ms\r\n", stack, timestamp - s.startTimestamp);
				StopDischargeInStack(timestamp, stack);
				break;
			}

			outputs.stacks[stack].dischargeEnableMask = s.dischargeEnableMask;
			break;
		}

		case StackBalancingState_t::finished:
			numFinishedStacks++;
			break;
		}
	}

	outputs.timeSinceLastDischarge = timeSinceLastDischarge.GetTimeElapsed();

	if (numFinishedStacks == kMaxStacks) {
		return BalancingAction_t::finish;
	}
	else {
		timeSinceLastDischarge.Restart();
		return BalancingAction_t::continue_;
	}
}

void Balancing::StopDischargeInStack(uint32_t timestamp, int stack) {
	stacks[stack].coolingDownMask = stacks[stack].dischargeEnableMask;

	for (int i = 0; i < kBMSCellChannels; i++) {
		if (stacks[stack].coolingDownMask & (1 << i)) {
			stacks[stack].coolingDownTimestamps[i] = timestamp;
		}
	}

	stacks[stack].state = StackBalancingState_t::cooldown;
	stacks[stack].dischargeEnableMask = 0;
	stacks[stack].startTimestamp = timestamp;
}
