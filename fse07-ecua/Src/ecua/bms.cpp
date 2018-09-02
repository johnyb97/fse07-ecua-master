/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#include "balancing.hpp"
#include "bms.hpp"
#include "bmstemperaturemap.hpp"
#include "ecua.hpp"
#include "ecua_options.hpp"
#include "peripherals.hpp"

#include <math.h>
#include <tx2/ringbuf.h>
#include "can_ECUA.h"

extern "C" {
#include "bq76.h"
}

struct StackTempMeasurementFrame {
	int16_t temps_degC[kBMSTempsPerStack];
	int16_t temps_degC_sort[kBMSTempsPerStack];
	int16_t analog_die_temp_degC;
	int16_t digital_die_temp_degC;

	int temp_median;
	int temp_high;

	int timestamp;
};

struct StackCellMeasurementFrame {
	Voltage_t voltages[kBMSCellChannels];
	Voltage_t voltage_min;
	Voltage_t voltage_max;
};

struct PackTempMeasurementFrame {
	StackTempMeasurementFrame stacks[kMaxStacks];

	int temp_avg;
	int temp_max;

	bool locked;
	bool valid;
};

struct PackCellMeasurementFrame {
	StackCellMeasurementFrame stacks[kMaxStacks];

	int timestamp;
	Voltage_t voltage_min;
	Voltage_t voltage_max;
	Voltage_t voltage_diff;

	bool locked;
	bool valid;
};

static Peripherals* s_peripherals;
static SDC* s_sdc;

static Balancing s_balancing;

static BMS::HighState stateHigh = BMS::HighState::fault;
static BMS::LowState stateLow = BMS::LowState::ready;
static SysTickTimer stateTimerHigh, stateTimerLow;
static BMS::FaultReason faultReason;

static int numErrors = 0;
static const int maxErrors = 2;

static int chain_len;
static int current_mux;
static int clamp_warnings = 0;

static int auxMeasurementMux; //FIXME

static SysTickTimer tempMeasTimer;
static PackTempMeasurementFrame tempMeas[3];
static int tempMeas_readIndex;
static int tempMeas_writeIndex;

static SysTickTimer cellMeasTimer;
static PackCellMeasurementFrame cellMeas[3];
static int cellMeas_readIndex;
static int cellMeas_writeIndex;

static std::optional<int>		s_lastMeasurementMaxTemp;
static std::optional<Voltage_t>	s_lastMeasurementMinCellVoltage;

static uint8_t rx_buffer[256];
static volatile ringbuf_t rx_rb;

static BmsTemperatureMap map;
static bool selfTestDone = false;

static const uint8_t init_seq[] = {
	BQ76_NCHAN, 		16,
	BQ76_GPIO_OUT, 		0x00,
	BQ76_GPIO_DIR, 		0x0F,
	BQ76_SMPL_DLY1, 	0x00,
	BQ76_CELL_SPER, 	0xBC,
	//AUX_SPER, ??,
	BQ76_OVERSMPL,		0x00,
	BQ76_CBCONFIG,		(1<<4)|0x08,
};

inline void MiscFault() {}

template <typename T>
T min(const T* array, size_t length) {
	T min_ = array[0];

	for (size_t i = 1; i < length; i++) {
		if (array[i] < min_)
			min_ = array[i];
	}

	return min_;
}

template <typename T>
T max(const T* array, size_t length) {
	T max_ = array[0];

	for (size_t i = 1; i < length; i++) {
		if (array[i] > max_)
			max_ = array[i];
	}

	return max_;
}

static int compar_int16_t(const void* a, const void* b) {
	return *(int16_t*)a - *(int16_t*)b;
}

static void s_CalculateStatistics(PackCellMeasurementFrame* frame) {
	for (int stack = 0; stack < chain_len - kUnStacks; stack++) {
		auto p_stack = &frame->stacks[stack];

		p_stack->voltage_max = max(p_stack->voltages, sizeof(p_stack->voltages) / sizeof(p_stack->voltages[0]));
		p_stack->voltage_min = min(p_stack->voltages, sizeof(p_stack->voltages) / sizeof(p_stack->voltages[0]));
	}

	Voltage_t voltage_min = frame->stacks[0].voltage_min;
	Voltage_t voltage_max = frame->stacks[0].voltage_max;

	for (int stack = 0; stack < chain_len - kUnStacks; stack++) {
		if (voltage_max < frame->stacks[stack].voltage_max) {
			voltage_max = frame->stacks[stack].voltage_max;
		}
		if (voltage_min > frame->stacks[stack].voltage_min) {
			voltage_min = frame->stacks[stack].voltage_min;
		}
	}

	frame->voltage_min = voltage_min;
	frame->voltage_max = voltage_max;
	frame->voltage_diff = voltage_max - voltage_min;
	s_lastMeasurementMinCellVoltage = voltage_min;
}

static void s_CalculateStatistics(PackTempMeasurementFrame* frame) {
	for (int stack = 0; stack < chain_len - kUnStacks; stack++) {
		qsort(&frame->stacks[stack].temps_degC_sort[0], kBMSValidTempsPerStack,
				sizeof(int16_t), compar_int16_t);

		frame->stacks[stack].temp_median = frame->stacks[stack].
				temps_degC_sort[kBMSValidTempsPerStack / 2];
		frame->stacks[stack].temp_high = frame->stacks[stack].
				temps_degC_sort[kBMSValidTempsPerStack * kBMSTempHighQuantile / 100];
	}

	int avgTemp = 0;
	int maxTemp = frame->stacks[0].temp_high;

	for (int stack = 0; stack < chain_len - kUnStacks; stack++) {
		avgTemp += frame->stacks[stack].temp_median;

		if (maxTemp < frame->stacks[stack].temp_high) {
			maxTemp = frame->stacks[stack].temp_high;
		}
	}

	frame->temp_avg = avgTemp / (chain_len - kUnStacks);
	frame->temp_max = maxTemp;
	s_lastMeasurementMaxTemp = maxTemp;
}

static uint8_t s_ConvertTemp(int temp_degC) {
	if (temp_degC < -25) {
		clamp_warnings++;
		return 0;
	}
	else if (temp_degC < 102) {
		return (int)((temp_degC + 25) * 2);
	}
	else {
		clamp_warnings++;
		return 255;
	}
}

static int8_t s_ToInt8(int temp_degC) {
	if (temp_degC < INT8_MIN) {
		clamp_warnings++;
		return INT8_MIN;
	}
	else if (temp_degC <= INT8_MAX) {
		return static_cast<int8_t>(temp_degC);
	}
	else {
		clamp_warnings++;
		return INT8_MAX;
	}
}

extern "C" int bmsReceiveCb(uint8_t* data_out) {
	uint32_t start = HAL_GetTick();

	while (HAL_GetTick() < start + 50) {
		size_t readpos = rx_rb.readpos;
		if (ringbufTryRead(&rx_rb, data_out, 1, &readpos)) {
			rx_rb.readpos = readpos;
			return 1;
		}
	}

	return 0;
}

#if 0
extern "C" void bq76ReadCellsAuxTempCb(int stack, uint16_t* cells_BE, uint16_t* aux_BE, uint16_t* temp_BE) {
	float voltage = Voltage_t::fromRaw(__ntohs(temp_BE[1])).toVolts();
	float analog_die_temp = (voltage-1.8078)*147.514;
	frame->analog_die_temp_degC = (int)analog_die_temp;

	voltage = Voltage_t::fromRaw(__ntohs(temp_BE[0])).toVolts();
	float digital_die_temp = (voltage-2.287)*131.944;
	frame->digital_die_temp_degC = digital_die_temp;
}
#endif

void BMS::Init(Peripherals* peripherals, SDC* sdc) {
	s_peripherals = peripherals;
	s_sdc = sdc;

	rx_rb.data = rx_buffer;
	rx_rb.size = sizeof(rx_buffer);
	rx_rb.readpos = 0;
	rx_rb.writepos = 0;

	if (!enableBMS) {
		return;
	}

	p_SetHighState(HighState::pendingInit);
}

void BMS::Restart() {
	if (!enableBMS) {
		return;
	}

	rx_rb.data = rx_buffer;
	rx_rb.size = sizeof(rx_buffer);
	rx_rb.readpos = 0;
	rx_rb.writepos = 0;

	p_SetHighState(HighState::pendingInit);
}

void BMS::FinishFrameCells() {
	cellMeas[cellMeas_readIndex].locked = false;
	cellMeas[cellMeas_readIndex].valid = false;
	cellMeas_readIndex = (cellMeas_readIndex + 1) % (sizeof(cellMeas) / sizeof(cellMeas[0]));
}

void BMS::FinishFrameTemps() {
	tempMeas[tempMeas_readIndex].locked = false;
	tempMeas[tempMeas_readIndex].valid = false;
	tempMeas_readIndex = (tempMeas_readIndex + 1) % (sizeof(tempMeas) / sizeof(tempMeas[0]));
}

std::optional<TemperatureRange> BMS::GetTemperatureRange() {
	int maxTemp_degC;

	if (!GetMaxTemp(&maxTemp_degC)) {
		return {};
	}
	else if (maxTemp_degC > kAcpCriticalTemp_degC) {
		return TemperatureRange::critical;
	}
	else if (maxTemp_degC > kAcpWarningTemp_degC) {
		return TemperatureRange::warning;
	}
	else {
		return TemperatureRange::normal;
	}
}

bool BMS::GetMaxTemp(int* maxTemp_degC) {
	if (s_lastMeasurementMaxTemp.has_value()) {
		*maxTemp_degC = *s_lastMeasurementMaxTemp;
		return true;
	}
	else {
		return false;
	}
}

bool BMS::GetMinCellVoltage(Voltage_t* voltMin_out) {
	if (s_lastMeasurementMinCellVoltage.has_value()) {
		*voltMin_out = *s_lastMeasurementMinCellVoltage;
		return true;
	}
	else {
		return false;
	}
}

int BMS::GetNumStacks() {
	if (chain_len < kUnStacks) {
		return 0;
	}
	else {
		return chain_len - kUnStacks;
	}
}

bool BMS::GetSummary(Voltage_t* voltMin_out, Voltage_t* voltMax_out, int8_t* maxTemp_degC_out) {
	{
		auto frame = &cellMeas[cellMeas_readIndex];

		if (!frame->valid) {
			return false;
		}
		else {
			frame->locked = true;
		}

		*voltMin_out = frame->voltage_min;
		*voltMax_out = frame->voltage_max;
	}
	{
		auto frame = &tempMeas[tempMeas_readIndex];

		if (!frame->valid) {
			return false;
		}
		else {
			frame->locked = true;
		}

		*maxTemp_degC_out = s_ToInt8(frame->temp_max);
	}

	return true;
}

bool BMS::GetTempAt(int stack, int index, uint8_t* temp_out) {
	if (map.Get(stack, index) == BmsTemperatureMap::Quality::bad) {
		*temp_out = 0;
		return true;
	}

	auto frame = &tempMeas[tempMeas_readIndex];

	if (!frame->valid) {
		return false;
	}
	else {
		frame->locked = true;
	}

	if (index >= 0 && index < 64) {
		*temp_out = s_ConvertTemp(frame->stacks[stack].temps_degC[index]);
		return true;
	}
	else if (index == 64) {
		*temp_out = s_ConvertTemp(frame->stacks[stack].analog_die_temp_degC);
		return true;
	}
	else if (index == 65) {
		*temp_out = s_ConvertTemp(frame->stacks[stack].digital_die_temp_degC);
		return true;
	}
	else {
		return false;
	}
}

bool BMS::GetVoltageAt(int stack, int cell, Voltage_t* voltage_out, int* timestamp_out) {
	auto frame = &cellMeas[cellMeas_readIndex];

	if (!frame->valid) {
		return false;
	}
	else {
		frame->locked = true;
	}

	*voltage_out = frame->stacks[stack].voltages[cell];
	*timestamp_out = frame->timestamp;
	return true;
}

bool BMS::IsFault() {
	return stateHigh == HighState::fault;
}

void BMS::p_ResetCommunication() {
	if (s_peripherals->GetEnvironment() != Environment::car) {
		s_sdc->Fault();
	}

	//bq76WriteBroadcast8(BQ76_GPIO_OUT, 0);
	bq76ShutdownAll();
	p_SetLowState(LowState::ready);
	p_SetHighState(HighState::communicationReset);
}

void BMS::p_SetFault(BMS::FaultReason reason) {
	stateHigh = HighState::fault;
	stateTimerHigh.Restart();
	faultReason = reason;
}

void BMS::p_SetLowState(BMS::LowState state) {
	stateLow = state;
	stateTimerLow.Restart();
}

void BMS::p_SetHighState(BMS::HighState state) {
	stateHigh = state;
	stateTimerHigh.Restart();
}

void BMS::p_SetStackIndicationLEDs(int stacks) {
	if (kUnStacks < 1) {
		return;
	}

	if (stacks == 0) {
		bq76Write8(0, BQ76_GPIO_OUT, 0x00);
	}
	else {
		int mask = (1<<stacks)-1;
		bq76Write8(0, BQ76_GPIO_OUT, mask);
	}
}

void BMS::p_StartCellMeasurement() {
	bq76MeasureCells();
	// begin readout
	bq76ReadoutRequest(chain_len);

	p_SetLowState(LowState::cellReadout);
}

void BMS::p_StartAuxMeasurement(int mux) {
	current_mux = mux;

	for (int device = kUnStacks; device < chain_len; device++) {
		bq76Write8(device, BQ76_GPIO_OUT, (current_mux<<1)|1);
	}

	p_SetLowState(LowState::settingMux);
}

/*void BMS::Suspend() {
	bq76ShutdownAll();
	p_SetLowState(LowState::suspend);
	p_SetHighState(HighState::suspend);
}*/

void BMS::i_UartReceiveData(uint8_t data) {
	if (!ringbufWrite(&rx_rb, &data, 1)) {
		MiscFault();
	}
}

void BMS::p_UpdateBalancingModel(PackCellMeasurementFrame* frame) {
	if (chain_len != kMaxStacks + kUnStacks || s_peripherals->GetEnvironment() != Environment::charger) {
		return;
	}

	static int lastUpdate = 0;

	auto timestamp = HAL_GetTick();

	if (timestamp - lastUpdate < 1000)
		return;

	lastUpdate = timestamp;
	printf("BALANCING UPDATE (t=%lu)\r\n", timestamp);

	// Step 1: gather input data and make sure it's valid
	Balancing::Inputs_t inputs;

	inputs.globalMinVoltage = frame->stacks[0].voltages[0];

	for (int stack = 0; stack < kMaxStacks; stack++) {
		for (int i = 0; i < kBMSCellChannels; i++) {
			inputs.stacks[stack].cellVoltages[i] = frame->stacks[stack].voltages[i];

			if (frame->stacks[stack].voltages[i] < inputs.globalMinVoltage) {
				inputs.globalMinVoltage = frame->stacks[stack].voltages[i];
			}
		}
	}

	inputs.allowedVoltageDifference = Voltage_t::fromMillivolts(10);

	// Step 2: Run the model
	Balancing::Outputs_t outputs;
	auto action = s_balancing.Update(timestamp, inputs, outputs);

	ECUA_BalancingStatus_t report;
	for (int stack = 0; stack < kMaxStacks; stack++) {
		int have = 0;
		for (int cell = 0; cell < kBMSCellChannels; cell++) {
			if (outputs.stacks[stack].dischargeEnableMask & (1 << cell)) {
				report.active[stack * 2 + have] = 1;
				report.cellIndex[stack * 2 + have] = cell;
				have++;
			}
		}
		for (; have < 2; have++) {
			report.active[stack * 2 + have] = 0;
		}

	}
	ECUA_send_BalancingStatus_s(&report);

	if (ECUA_BalancingStatus2_need_to_send()) {
		ECUA_send_BalancingStatus2(outputs.targetCellVoltage.toTenthsMillivolt(),
								   inputs.allowedVoltageDifference.toTenthsMillivolt(),
								   outputs.timeSinceLastDischarge < UINT16_MAX ? outputs.timeSinceLastDischarge : UINT16_MAX);
	}

	// Step 3: Check results and take appropriate action
	if (action == BalancingAction_t::continue_) {
		for (int stack = 0; stack < kMaxStacks; stack++) {
			auto mask = outputs.stacks[stack].dischargeEnableMask;
			bq76Write16(kUnStacks + stack, BQ76_CBENBL, mask);
		}
	}
	else {
		bq76WriteBroadcast16(BQ76_CBENBL, 0);
	}
}

void BMS::p_UpdateHigh() {
	switch (stateHigh) {
	case HighState::pendingInit: {
		//bq76Write8(0, BQ76_DEV_CTRL, BQ76_DEV_CTRL_PWRDN);
		bq76ShutdownAll();
		HAL_Delay(100);

		bmsWkup.Set();
		HAL_Delay(50);
		bmsWkup.Clear();

		rx_rb.readpos = rx_rb.writepos;

		if (bq76DetectChainLength(&chain_len) < 0 || chain_len < kUnStacks) {
			p_SetFault(BMS::FaultReason::baseCommError);
			return;
		}

		// 1 second communication power down timeout
		// Legenda pravi, ze LittleHill nastavuje timeout zaporny, nebot prikazy posila z budoucnosti
		// Zda je jeho pristup spravny, se teprve ukaze
		bq76WriteBroadcast8(BQ76_CTO, (3<<4));

		// 30 seconds communication power down timeout
		//bq76WriteBroadcast8(BQ76_CTO, (7<<4));

		for (size_t i = 0; i < sizeof(init_seq); i += 2) {
			if (bq76WriteBroadcast8(init_seq[i], init_seq[i + 1]) < 0) {
				p_SetFault(BMS::FaultReason::commError);
				return;
			}
		}

		bq76Write8(0, BQ76_GPIO_DIR, 0x3F);

		// Init anim
		for (int i = 1; i <= chain_len-kUnStacks; i++) {
			p_SetStackIndicationLEDs(i);
			HAL_Delay(10);
		}

		if (enableBMSChainLengthCheck && chain_len != kBMSExpectedChainLength) {
			p_SetFault(BMS::FaultReason::chainLengthMismatch);
			return;
		}

		current_mux = 0;
		numErrors = 0;

		for (int i = 0; i < 3; i++) {
			cellMeas[i].locked = false;
			cellMeas[i].valid = false;
			tempMeas[i].locked = false;
			tempMeas[i].valid = false;
		}

		cellMeas_readIndex = 0;
		cellMeas_writeIndex = 0;
		tempMeas_readIndex = 0;
		tempMeas_writeIndex = 0;

		p_SetLowState(LowState::ready);
		if (!selfTestDone) {
			p_SetHighState(HighState::pendingSelfTest);
		}
		else {
			p_SetHighState(HighState::ready);
		}
		break;
	}

	case HighState::pendingSelfTest:
		if (stateLow == LowState::ready) {
			cellMeas_writeIndex = 0;
			tempMeas_writeIndex = 0;

			// start self-test
			auxMeasurementMux = 0;
			p_StartAuxMeasurement(auxMeasurementMux);

			p_SetHighState(HighState::selfTest);
		}
		break;

	case HighState::selfTest:
		if (stateLow == LowState::auxMeasurementFinished) {
			if (auxMeasurementMux + 1 < kBMSNumMux) {
				// there are more measurements to do
				p_StartAuxMeasurement(++auxMeasurementMux);
			}
			else {
				// TODO: which frame was written?
				auto frame = &tempMeas[tempMeas_writeIndex];

				// calculate median
				s_CalculateStatistics(frame);
				auto median = frame->temp_avg;

				int threshold_good = 5;
				int threshold_suspect = 10;
				int median_min_calib = 15;
				int median_max_calib = 30;

				map.Clear(BmsTemperatureMap::Quality::unknown);

				if (median >= median_min_calib && median <= median_max_calib) {
					for (int stack = 0; stack < chain_len - kUnStacks; stack++) {
						for (int index = 0; index < kBMSValidTempsPerStack; index++) {
							int dev = abs(frame->stacks[stack].temps_degC[index] - median);
							if (dev < threshold_good) {
								map.Put(stack, index, BmsTemperatureMap::Quality::good);
							}
							else if (dev < threshold_suspect) {
								map.Put(stack, index, BmsTemperatureMap::Quality::suspicious);
							}
							else {
								map.Put(stack, index, BmsTemperatureMap::Quality::bad);
							}
						}
					}

					map.SaveToFlash(pBMSFlashAddr);
				}
				else if (map.LoadFromFlash(pBMSFlashAddr)) {
				}
				else {
					map.Clear(BmsTemperatureMap::Quality::suspicious);
				}

				selfTestDone = true;
				p_SetHighState(HighState::ready);
			}
		}
		break;

	case HighState::ready:
		// less than 100ms left to measure cells?
		if (cellMeasTimer.TimeElapsed(tBMSCellMeasInterval)) {
			if (cellMeas[cellMeas_writeIndex].locked) {
				// if locked for reading (never more than one frame at a time), grab the next one
				cellMeas_writeIndex = (cellMeas_writeIndex + 1) % (sizeof(cellMeas) / sizeof(cellMeas[0]));
			}

			cellMeas[cellMeas_writeIndex].valid = false;

			// begin measurement
			p_StartCellMeasurement();
			p_SetHighState(HighState::measuring);
		}
		else if (tempMeasTimer.TimeElapsed(tBMSTempMeasInterval)) {
			if (auxMeasurementMux == 0) {
				if (tempMeas[tempMeas_writeIndex].locked) {
					// if locked for reading (never more than one frame at a time), grab the next one
					tempMeas_writeIndex = (tempMeas_writeIndex + 1) % (sizeof(tempMeas) / sizeof(tempMeas[0]));
				}

				tempMeas[tempMeas_writeIndex].valid = false;
			}

			// begin measurement
			p_StartAuxMeasurement(auxMeasurementMux);
			p_SetHighState(HighState::measuring);
		}
		break;

	case HighState::measuring:
		if (stateLow == LowState::cellMeasurementFinished) {
			auto frame = &cellMeas[cellMeas_writeIndex];
			s_CalculateStatistics(frame);
			// TODO: check for fault events

			frame->valid = true;
			cellMeas_writeIndex = (cellMeas_writeIndex + 1) % (sizeof(cellMeas) / sizeof(cellMeas[0]));

			p_SetHighState(HighState::ready);
			cellMeasTimer.Restart();
		}
		else if (stateLow == LowState::auxMeasurementFinished) {
			// TODO: check for fault events

			if (auxMeasurementMux + 1 == kBMSNumMux) {
				auto frame = &tempMeas[tempMeas_writeIndex];
				s_CalculateStatistics(frame);

				frame->valid = true;
				tempMeas_writeIndex = (tempMeas_writeIndex + 1) % (sizeof(tempMeas) / sizeof(tempMeas[0]));

				auxMeasurementMux = 0;
			}
			else {
				auxMeasurementMux++;
			}

			p_SetHighState(HighState::ready);
			tempMeasTimer.Restart();
		}
		break;

	case HighState::communicationReset:
		if (stateTimerHigh.TimeElapsed(tBMSCommunicationResetDelay)) {
			p_SetHighState(HighState::pendingInit);
		}
		break;

	case HighState::fault:
		break;
	}
}

void BMS::p_UpdateLow() {
	switch (stateLow) {
	case LowState::ready:
		break;

	case LowState::cellReadout: {
		constexpr size_t frame_length = (1 + kBMSCellChannels*2 + 2);	// FIXME: don't do this shit here
		const size_t total_length = chain_len * frame_length;

		if (ringbufCanRead(&rx_rb, total_length)) {
			uint16_t cell_buffer[kMaxStacks * kBMSCellChannels];
			int rc = bq76CellReadoutRecv(chain_len, kUnStacks, chain_len-kUnStacks, kBMSCellChannels, cell_buffer);

			if (rc < 0) {
				p_ResetCommunication();
				break;
			}

			auto frame = &cellMeas[cellMeas_writeIndex];
			frame->timestamp = HAL_GetTick();	// TODO: ok?

			for (int stack = 0; stack < chain_len-kUnStacks; stack++) {
				for (int i = 0; i < kBMSCellChannels; i++) {
					// FIXME: can this overflow?
					auto frame = &cellMeas[cellMeas_writeIndex].stacks[stack];

					frame->voltages[i] = Voltage_t::fromRaw(cell_buffer[stack * kBMSCellChannels + i]);
				}
			}

			p_SetLowState(LowState::cellMeasurementFinished);
			if (enableBalancing) {
				p_UpdateBalancingModel(frame);
			}
			numErrors = 0;
		}
		else if (stateTimerHigh.TimeElapsed(tBMSReadoutTimeout)) {
			if (++numErrors < maxErrors) {
				p_StartCellMeasurement();
			}
			else {
				p_ResetCommunication();
			}
		}
		break;
	}

	case LowState::settingMux:
		// AUX sampling strategy:
		// - configure mux & wait to settle
		// - broadcast SYNCHRONOUSLY SAMPLE CHANNELS
		// - turn off LDO
		// - broadcast READ SAMPLED VALUES

		if (stateTimerLow.TimeElapsed(tBMSAuxSettle)) {
			// sample the aux channels
			bq76MeasureAux();

			p_SetLowState(LowState::auxSampling);
		}
		break;

	case LowState::auxSampling:
		if (stateTimerLow.TimeElapsed(tBMSAuxSample)) {
			// turn off NTC LDO to conserve power
			for (int device = kUnStacks; device < chain_len; device++) {
				bq76Write8(device, BQ76_GPIO_OUT, (current_mux<<1)|0);
			}

			// begin readout
			bq76ReadoutRequest(chain_len);

			p_SetLowState(LowState::auxReadout);
		}
		break;

	case LowState::auxReadout: {
		constexpr size_t frame_length = (1 + kBMSAuxChannels*2 + 2);	// FIXME: don't do this shit here
		const size_t total_length = chain_len * frame_length;

		if (ringbufCanRead(&rx_rb, total_length)) {
			uint16_t aux_buffer[kMaxStacks * kBMSAuxChannels];
			int rc = bq76AuxReadoutRecv(chain_len, kUnStacks, chain_len-kUnStacks, kBMSAuxChannels, aux_buffer);

			if (rc < 0) {
				p_ResetCommunication();
				break;
			}

			for (int stack = 0; stack < chain_len-kUnStacks; stack++) {
				for (int i = 0; i < kBMSAuxChannels; i++) {
					// FIXME: can this overflow?
					auto frame = &tempMeas[tempMeas_writeIndex].stacks[stack];

					float voltage = Voltage_t::fromRaw(aux_buffer[stack * kBMSAuxChannels + i]).toVolts();
					float aux_R = ((5.1*4700)/voltage)-4700;
					float temp = 1/(0.000872515609214041+0.000254103069629347*log(aux_R)+0.000000180594564401951*pow(log(aux_R), 3))-273.16;

					//s_stackData[stack - 1].temps[current_mux * 8 + i] = temp;
					frame->temps_degC[current_mux * 8 + i] = (int)temp;
					frame->temps_degC_sort[current_mux * 8 + i] = (int)temp;
				}
			}

			// TODO: timestamp

			p_SetLowState(LowState::auxMeasurementFinished);
			numErrors = 0;
		}
		else if (stateTimerHigh.TimeElapsed(tBMSReadoutTimeout)) {
			if (++numErrors < maxErrors) {
				bq76ReadoutRequest(chain_len);

				p_SetLowState(LowState::auxReadout);
			}
			else {
				p_ResetCommunication();
			}
			break;
		}
		break;
	}

	case LowState::auxMeasurementFinished:
	case LowState::cellMeasurementFinished:
		// No-op
		break;
	}
}

void BMS::Update() {
	p_UpdateLow();
	p_UpdateHigh();
}
