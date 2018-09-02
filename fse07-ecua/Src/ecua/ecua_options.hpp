#pragma once

#include "types.hpp"

#include <stdio.h>
#define debug_printf(args) printf args

// Configuration options

enum { enableBMS = 1 };
enum { enableBMSChainLengthCheck = 1 };
enum { enableCellVoltageStartCheck = 1 };
enum { enableCellVoltageOperationCheck = 0 };
enum { enableTemperatureStartCheck = 1 };
enum { enableTemperatureOperationCheck = 1 };
enum { enableBalancing = 1 };

enum { enableHVDCRelay = 1 };
enum { enableHVDCGlv = 1 };
enum { enableHVDCFans = 1 };

// All time constants are in milliseconds
enum {
	// State machine timing
	tPrechargeCloseTimeout = 20,
	tPrechargeAccu = 700,
	tPrechargeCharger = 2000,
	tPrechargeCooldown = 2000,
	tAirMCloseTimeout = 150,
	tAirPCloseTimeout = 150,
	tAirMCooldown = 500,
	tPrechargeSlopeTimeout = 1000,
	tPrechargeTimeout = 5000,

	// SDC timing
	tIMDStartup = 4000,

	// Measurement timing
	tVoltageSensePeriod = 100,

	// ACP configuration, BMS options
	kMaxStacks = 6,
	kUnStacks = 0,
	kBMSCellChannels = 16,
	kBMSAuxChannels = 8,
	kCellCutoffVoltage = 2500,
	kCellMaxVoltage = 4250,

	kBMSValidTempsPerStack = 64,
	kBMSTempsPerStack = 66,
	kBMSNumMux = 8,

	kBMSExpectedChainLength = kMaxStacks + kUnStacks,

	tBMSTempMeasInterval = 600,		// 4.8 sec to scan all -> must send every 72 ms to keep up
	tBMSCellMeasInterval = 1000,

	tBMSReadoutTimeout = 500,
	tBMSCommunicationResetDelay = 1000,

	tBMSAuxSettle = 20,
	tBMSAuxSample = 2,

	kBMSTempHighQuantile = 85,

	pBMSFlashAddr = 0x0800FC00,

	// Peripherals
	kDcdcEnableDuringCharging = 0,
	tDcdcSoftStart = 15000,
	tDcdcEnableInterval = 1000,
	kDcdcMinInputVolts = 180,
};

// Never discharge below this valie when balancing
constexpr Voltage_t kBalancingMinTargetVoltage = Voltage_t::fromMillivolts(3500);

enum {
	kMaxErrors = 5,
	kVdiffMax = 15,						// max. voltage difference ACCU-AIRs (volts)
	//kMinCellVoltage = 25000,
	//kMaxCellVoltageDifference = 150,
	//kMaxCellTemperature = 80,
	//kBMSMaxCellTempDiffFromAverage = 10,

	kAcpCutoffVoltage_mV = 192000,
	kAcpFullVoltage_mV = 410000,

	kAcpUVThreshold_mV = 180000,
	kAcpOVThreshold_mV = 415000,

	kAcpWarningTemp_degC = 55,
	kAcpCriticalTemp_degC = 80,
};

// Peripherals
enum {
	// Speed in range 0..1000 (0-100%)
	kFansMaxSpeed = 1000,
	kFansActiveSpeed = 200,
	kFansIdleSpeed = 0,

	kFansActiveTemp_degC = 30,
	kFansMaxTemp_degC = 55,

	tFansUpdateInterval = 100,
	kFansRampSlope = 5,
};

/*enum {
	kEventBoot = 0x01,
	kEventInitOK = 0x02,
	kEventStateTransition = 0x03,
	kEventCellMinVoltage = 0x04,
	kEventCellImbalance = 0x05,
	kEventCellTemperature = 0x06,
};*/
