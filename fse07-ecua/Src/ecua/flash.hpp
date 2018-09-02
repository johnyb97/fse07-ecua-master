#pragma once

#include <stddef.h>
#include <stdint.h>

class Flash {
public:
	static bool ErasePage(uint32_t pageAddress);
	static bool Write(uint32_t flashAddress, const void* data, size_t length);
};
