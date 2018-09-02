#pragma once

#include "base.hpp"

constexpr int kSysTickFrequency = 1000;

class SysTickTimer {
public:
	void Restart() {
		startTime = HAL_GetTick();
	}

	uint32_t GetTimeElapsed() {
		return HAL_GetTick() - startTime;
	}

	bool TimeElapsed(uint32_t millis) {
		return HAL_GetTick() > startTime + millis;
	}

	bool RestartIfTimeElapsed(uint32_t millis) {
		if (HAL_GetTick() > startTime + millis) {
			this->Restart();
			return true;
		}
		else {
			return false;
		}
	}

private:
	uint32_t startTime;
};

class Timeout {
public:
	void Init() {
		timer.Restart();
	}

	bool TryConsume(uint32_t millis) {
		if (!consumed && timer.TimeElapsed(millis)) {
			consumed = true;
			return true;
		}
		else
			return false;
	}

private:
	SysTickTimer timer;
	bool consumed = false;
};
