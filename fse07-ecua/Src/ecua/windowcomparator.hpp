/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#ifndef ecua_windowcomparator_hpp
#define ecua_windowcomparator_hpp

template <typename T, size_t length>
class WindowComparator {
public:
	bool AreAllValuesAbove(T threshold) {
		for (size_t i = 0; i < length; i++) {
			if (this->window[i] < threshold) {
				return false;
			}
		}

		return true;
	}

	void PushValue(T value) {
		this->window[index] = value;
		index = (index + 1) % length;
	}

private:
	T window[length] = {0};
	size_t index;
};

#endif
