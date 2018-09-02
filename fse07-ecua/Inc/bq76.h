#ifndef bq76_h
#define bq76_h

#include <stddef.h>
#include <stdint.h>

enum {
    BQ76_CMD = 0x02,
    BQ76_OVERSMPL = 0x07,
    BQ76_DEV_CTRL = 0x0C,
    BQ76_NCHAN = 0x0D,
	BQ76_CBCONFIG = 0x13,
	BQ76_CBENBL = 0x14,
    BQ76_CTO = 0x28,
    BQ76_SMPL_DLY1 = 0x3D,
    BQ76_CELL_SPER = 0x3E,
    BQ76_GPIO_DIR = 0x78,
    BQ76_GPIO_OUT = 0x79,
};

enum {
    BQ76_DEV_CTRL_PWRDN = 0x40
};

enum {
    BQ76_OK             = 0,
    BQ76_ERROR          = -1,
    BQ76_FRAME_ERROR    = -2
};

// CALLBACKS

void bmsDelayMsCb(int milliseconds);
void bmsTransmitCb(uint8_t data);
int bmsReceiveCb(uint8_t* data_out);

// FUNCTIONS

int bq76DetectChainLength(int* chain_length_out);

int bq76Write8(int device, uint8_t reg, uint8_t value);
int bq76Write16(uint8_t device, uint8_t reg, uint16_t value);
int bq76WriteBroadcast8(uint8_t reg, uint8_t value);
int bq76WriteBroadcast16(uint8_t reg, uint16_t value);

void bq76ShutdownAll(void);

int bq76MeasureCells(void);
int bq76MeasureAux(void);
int bq76MeasureCellsAuxTemp(void);

int bq76ReadoutRequest(int chain_len);
int bq76CellReadoutRecv(int chain_len, int first_device, int num_devices, size_t cells_per_device, uint16_t* cells_out);
int bq76AuxReadoutRecv(int chain_len, int first_device, int num_devices, size_t aux_channels_per_device, uint16_t* aux_out);

#endif
