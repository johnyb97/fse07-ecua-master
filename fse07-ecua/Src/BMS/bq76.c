#include "bq76.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "bq76_crc.h"

#define CHANNELS	0x03

#define ADDR		0x0A
#define NCHAN		0x0D
#define DEVCONFIG	0x0E
#define COMCONFIG	0x10


#define FAULT_SUM 	0x52
#define FAULT_SYS 	0x60
#define GPIO_DIR	0x78
#define GPIO_OUT	0x79
#define GPIO_IN		0x7C

enum { DEVCONFIG_ADDR_SEL =				0x10 };
enum { DEV_CTRL_AUTO_ADDRESS = 			0x08 };


static void transmitMany(const uint8_t* data, size_t count) {
	while (count--) {
		bmsTransmitCb(*(data++));
	}
}

static int receiveMany(uint8_t* data, size_t count) {
	while (count--) {
		if (!bmsReceiveCb(data))
			return 0;

		data++;
	}
	return 1;
}

static void TransmitWithCRC(const uint8_t* data, size_t size) {
	transmitMany(data, size);

	int crc = bq76Crc16(data, size);
	bmsTransmitCb(crc & 0xff);
	bmsTransmitCb(crc >> 8);
}

static int Read8(uint8_t device, uint8_t reg, uint8_t* value_out) {
	uint8_t data[4];

	data[0] = 0x81; data[1] = device; data[2] = reg; data[3] = 0;
	TransmitWithCRC(data, 4);

	if (receiveMany(data, 1 + 1 + 2)) {
		*value_out = data[1];
		return 1;
	}
	else {
		return 0;
	}
}

static int bmsRead16(uint8_t device, uint8_t reg) {
	uint8_t data[5];

	data[0] = 0x81; data[1] = device; data[2] = reg; data[3] = 1;
	TransmitWithCRC(data, 4);

	receiveMany(data, 1 + 2 + 2);
	return data[1] * 256 + data[2];
}

static int Write8(uint8_t device, uint8_t reg, uint8_t value) {
	uint8_t data[4];
	data[0] = 0x91; data[1] = device; data[2] = reg; data[3] = value;
	TransmitWithCRC(data, 4);
	return 0;
}

static int WriteBroadcast8(uint8_t reg, uint8_t value) {
	uint8_t data[3];
	data[0] = 0xF1; data[1] = reg; data[2] = value;
	TransmitWithCRC(data, 3);
	return 0;
}

static int WriteBroadcastWithResponse8(uint8_t reg, uint8_t value) {
	uint8_t data[3];
	data[0] = 0xE1; data[1] = reg; data[2] = value;
	TransmitWithCRC(data, 3);
	return 0;
}

int bq76Write8(int device, uint8_t reg, uint8_t value) {
	return Write8(device, reg, value);
}

int bq76Write16(uint8_t device, uint8_t reg, uint16_t value) {
	uint8_t data[5];
	data[0] = 0x92; data[1] = device; data[2] = reg; data[3] = (value >> 8); data[4] = (value & 0xff);
	TransmitWithCRC(data, 5);
	return 0;
}

int bq76WriteBroadcast8(uint8_t reg, uint8_t value) {
	return WriteBroadcast8(reg, value);
}

int bq76WriteBroadcast16(uint8_t reg, uint16_t value) {
	uint8_t data[4];
	data[0] = 0xF2; data[1] = reg; data[2] = (value >> 8); data[3] = (value & 0xff);
	TransmitWithCRC(data, 4);
	return 0;
}

// output data size is 8 * uint16_t
/*void bmsReadAuxValues(uint16_t* data_out) {
	//uint8_t data[2 * (1 + (16+8+2) * 2 + 2)];
	uint8_t data[2 * (1 + (8+2) * 2 + 2)];

	// channel selection
	data[0] = 0xF4; data[1] = 0x03; data[2] = 0x00; data[3] = 0x00; data[4] = 0xFF; data[5] = 0xC0;
	TransmitWithCRC(data, 6);

	// read data
	data[0] = 0xE1; data[1] = 0x02; data[2] = 0x01;
	TransmitWithCRC(data, 3);

	receiveMany(data, 2 * (1 + (8+2) * 2 + 2));

	for (int i = 0; i < 8; i++) {
		data_out[7 - i] = data[1 + i * 2] * 256 + data[1 + i * 2 + 1];
	}
}*/

int bq76DetectChainLength(int* chain_length_out) {
	// TODO: try to remove delay
	const int DLY = 5;

	// Configure communications uniformly (per slva617a 1.2.1)
	bq76WriteBroadcast16(COMCONFIG, 0x10e0);
	bmsDelayMsCb(DLY);

	// Enter auto-addressing mode
	WriteBroadcast8(DEVCONFIG, DEVCONFIG_ADDR_SEL);
	bmsDelayMsCb(DLY);

	WriteBroadcast8(BQ76_DEV_CTRL, DEV_CTRL_AUTO_ADDRESS);
	bmsDelayMsCb(DLY);

	// Set adresses of all BMS
	for (int i = 0; i < 16; i++) {
		WriteBroadcast8(ADDR, i);
		bmsDelayMsCb(DLY);
	}

	// Detect how many BMS respond
	int chain_len;
	for (chain_len = 0; chain_len < 16; chain_len++) {
		uint8_t addr;

		if (!Read8(chain_len, ADDR, &addr)) {
			break;
		}
	}

	// Disable High-Side Receiver on Differential Interface on Top of Stack
	bq76Write16(chain_len - 1, COMCONFIG, 0x1020);

	// Disable Low-Side Transmitter on Differential Interface on Bottom of Stack
	bq76Write16(0, COMCONFIG, 0x10C0);

	// Clear All Faults
	for (int i = chain_len - 1; i >= 0; i--) {
		bq76Write16(i, 0x52, 0xFFC0);
	}

	*chain_length_out = chain_len;
	return chain_len;
}

int bq76MeasureCells(void) {
	uint8_t data[6];

	// channel selection cells
	data[0] = 0xF4;
	data[1] = CHANNELS;
	data[2] = 0xFF;
	data[3] = 0xFF;
	data[4] = 0x00;
	data[5] = 0x00;
	TransmitWithCRC(data, 6);

	// sample data
	data[0] = 0xF1;
	data[1] = BQ76_CMD;
	data[2] = 0x00;
	TransmitWithCRC(data, 3);

	return BQ76_OK;
}

int bq76MeasureAux(void) {
	uint8_t data[6];

	// channel selection aux + temperatures
	data[0] = 0xF4;
	data[1] = CHANNELS;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0xFF;
	data[5] = 0x00;
	TransmitWithCRC(data, 6);

	// sample data
	data[0] = 0xF1;
	data[1] = BQ76_CMD;
	data[2] = 0x00;
	TransmitWithCRC(data, 3);

	return BQ76_OK;
}

int bq76MeasureCellsAuxTemp(void) {
	uint8_t data[7];

	// channel selection voltages + aux +  temperatures
	data[0] = 0xF4;
	data[1] = CHANNELS;
	data[2] = 0xFF;
	data[3] = 0xFF;
	data[4] = 0xFF;
	data[5] = 0xC0;
	TransmitWithCRC(data, 6);

	// sample data
	data[0] = 0xF1;
	data[1] = BQ76_CMD;
	data[2] = 0x00;
	TransmitWithCRC(data, 3);

	return BQ76_OK;
}

int bq76ReadoutRequest(int chain_len) {
	return WriteBroadcastWithResponse8(BQ76_CMD, 0x20 | (chain_len - 1));
}

int bq76CellReadoutRecv(int chain_len, int first_device, int num_devices, size_t cells_per_device, uint16_t* cells_out) {
	for (int device = chain_len - 1; device >= 0; device--) {
		uint8_t header;
		if (!bmsReceiveCb(&header) || header != 2 * cells_per_device - 1) {
			return BQ76_FRAME_ERROR;
		}

		for (int i = 0; i < cells_per_device; i++) {
			uint8_t bytes[2];

			if (!receiveMany(bytes, 2)) {
				return BQ76_FRAME_ERROR;
			}

			if (device >= first_device && device < first_device + num_devices) {
				// big-endian to native
				cells_out[(device - first_device) * cells_per_device + (cells_per_device - i - 1)] = bytes[0] * 256 + bytes[1];
			}
		}

		uint8_t crc_bytes[2];

		if (!receiveMany(crc_bytes, 2)) {
			return BQ76_FRAME_ERROR;
		}
	}

	return BQ76_OK;
}

int bq76AuxReadoutRecv(int chain_len, int first_device, int num_devices, size_t num_aux, uint16_t* aux_out) {
	for (int device = chain_len - 1; device >= 0; device--) {
		uint8_t header;
		if (!bmsReceiveCb(&header) || header != 2 * num_aux - 1) {
			return BQ76_FRAME_ERROR;
		}

		for (int i = 0; i < num_aux; i++) {
			uint8_t bytes[2];

			if (!receiveMany(bytes, 2)) {
				return BQ76_FRAME_ERROR;
			}

			if (device >= first_device && device < first_device + num_devices) {
				// big-endian to native
				aux_out[(device - first_device) * num_aux + (num_aux - i - 1)] = bytes[0] * 256 + bytes[1];
			}
		}

		uint8_t crc_bytes[2];

		if (!receiveMany(crc_bytes, 2)) {
			return BQ76_FRAME_ERROR;
		}
	}

	return BQ76_OK;
}

void bq76ShutdownAll(void) {
	WriteBroadcast8(BQ76_DEV_CTRL, BQ76_DEV_CTRL_PWRDN);
}
