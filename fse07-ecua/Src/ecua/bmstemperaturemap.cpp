/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */

#include "bmstemperaturemap.hpp"
#include "flash.hpp"

#include <string.h>

extern "C" {
#include "bq76_crc.h"
}

struct Header {
	char magic[4];
	uint16_t length;
	uint16_t checksum;
};

static_assert(sizeof(Header) == 8);

void BmsTemperatureMap::Clear(Quality quality) {
	const uint8_t fill = int(quality) | (int(quality) << 2) | (int(quality) << 4) | (int(quality) << 6);

	for (size_t stack = 0; stack < kMaxStacks; stack++) {
		for (size_t row = 0; row < rows; row++) {
			this->data[stack][row] = fill;
		}
	}
}

BmsTemperatureMap::Quality BmsTemperatureMap::Get(int stack, int index) {
	const size_t byte = index / 4;
	const int shift = (index % 4) * 2;
	const int mask = (0b11 << shift);

	return (Quality)((this->data[stack][byte] & mask) >> shift);
}

bool BmsTemperatureMap::LoadFromFlash(uint32_t address) {
	const uint8_t* h_buf = (const uint8_t*) address;

	Header h;
	memcpy(&h, &h_buf[0], sizeof(h));

	volatile uint16_t crc = bq76Crc16((const uint8_t*) address + sizeof(h), h.length);

	if (memcmp(h.magic, "BMS1", 4) != 0 || h.length != sizeof(this->data) || h.checksum != crc) {
		return false;
	}

	memcpy(&this->data, (const uint8_t*) address + sizeof(h), sizeof(this->data));
	return true;
}

void BmsTemperatureMap::Put(int stack, int index, Quality quality) {
	const size_t byte = index / 4;
	const int shift = (index % 4) * 2;
	const int mask = (0b11 << shift);

	this->data[stack][byte] = (this->data[stack][byte] & ~mask) | (int(quality) << shift);
}

void BmsTemperatureMap::SaveToFlash(uint32_t address) {
	uint16_t crc = bq76Crc16((const uint8_t*) &this->data, sizeof(this->data));

	Header h {'B', 'M', 'S', '1', sizeof(this->data), crc};

	uint8_t h_buf[sizeof(h)];
	memcpy(&h_buf[0], &h, sizeof(h));

	Flash::ErasePage(address);
	Flash::Write(address, h_buf, sizeof(h_buf));
	Flash::Write(address + sizeof(h_buf), this->data, sizeof(this->data));
}
