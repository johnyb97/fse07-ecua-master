/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#ifndef ecua_peripherals_hpp
#define ecua_peripherals_hpp

#include "voltagesense.hpp"
#include "windowcomparator.hpp"

enum class DcdcState {
	off,
	precharging,
	//restartBms,
	started1,
	startedAll,

	disable,
	disabling,
};

enum class Environment {
	car,
	charger,
	unknown,
};

class Peripherals {
public:
	Peripherals(VoltageSense* voltages) : voltages(voltages) {}

	void Init();
	void Update();

	Environment GetEnvironment() const { return environment; }
	bool AreFansActive() { return fansActive; }
	bool IsDcdcGlvActive() { return glvActive; }

	void SetEnvironment(Environment env);

private:
	VoltageSense* voltages;

	Environment environment = Environment::unknown;
	DcdcState dcdcState = DcdcState::off;
	SysTickTimer dcdcPrechargeTimer;
	bool fansActive = false;
	bool glvActive = false;

	// DCDC has a minimum voltage value for safe operation. This ensures it is met.
	WindowComparator<int, 5> hvInVolts;
	SysTickTimer hvInVoltsSampleTimer;
};

#endif
