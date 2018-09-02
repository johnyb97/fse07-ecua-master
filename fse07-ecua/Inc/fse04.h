#pragma once

#include <stdint.h>

enum {
	ECUA4V1_UID = 0x19,
	ECUP_UID = 6,
	DRS_UID = 7,
};

#define MAKE_SID_CTRL(dev_id_) (80 + (dev_id_))
#define MAKE_SID_STATUS(dev_id_) (96 + (dev_id_))
#define MAKE_SID_LINK(dev_id_) (128 + (dev_id_) * 8)

#define MAKE_CAN_ID(dev_id, msg, priority) (priority * 256 + (dev_id - 1) * 16 + (msg - 1))

enum {
	// 1MBit
	/*FSE04_CNF1 = 0x00,
	FSE04_CNF2 = 0x90,
	FSE04_CNF3 = 0x02,*/

	// 500kbit
	/*FSE04_CNF1 = 0x00,
	FSE04_CNF2 = 0xB1,
	FSE04_CNF3 = 0x05,*/

	// 250kbit
	FSE04_CNF1 = 0x01,
	FSE04_CNF2 = 0xAC,
	FSE04_CNF3 = 0x03,
};

enum {
	CTRL_SET_STATUS_FUNCTION = 1,
};

enum {
	STATUS_SL = 0x02,
	STATUS_DATAFIELDS = 0x20,
	STATUS_CAL = 0x40,
	STATUS_FWU = 0x80,
};

enum {
	SID_ECUA_AIRS_STATUS = 1058,		// DEV3_MSG3_P4
	SID_ECUA_BMS_BALANCING_ENABLE = 1065,	// DEV3_MSG10_P4
	SID_ECUA_BMS_CONTROL = 1060,		// DEV3_MSG5_P4
	SID_ECUA_BMS_TEMPS = 1061,			// DEV3_MSG6_P4
	SID_ECUA_BMS_VOLTAGES = 1062,		// DEV3_MSG7_P4
	SID_ECUA_CHARGER_HEARTBEAT = 1064,	// DEV3_MSG9_P4
	SID_ECUA_HIGHEST_TEMPS = 1063,		// DEV3_MSG8_P4
	SID_ECUA_RESET = 1059,				// DEV3_MSG4_P4
	SID_ECUA_SDC = 1056,				// DEV3_MSG1_P4
	SID_ECUA_START_BALANCING = 1066,	// DEV3_MSG11_P4
	SID_ECUA_STOP_BALANCING = 1067,		// DEV3_MSG12_P4
	SID_ECUA_VOLTAGES = 1057,			// DEV3_MSG2_P4

	SID_ECUB_STATUS = 1088,				// DEV5_MSG1_P4

	SID_ECUF_DIFF_CONFIG = 1074,		// DEV4_MSG3_P4
	SID_ECUF_STEERING = 1073,			// DEV4_MSG2_P4

	SID_ECUP_PEDALS = 873,				// DEV7_MSG10_P3
	SID_ECUP_DIAG = 1127,				// DEV7_MSG8_P4
	SID_ECUP_SDC = 1128,				// DEV7_MSG9_P4
	SID_ECUP_CALIB_REQUEST = 1638,		// DEV7_MSG7_P6
	SID_ECUP_THROTTLE = 613,	// DEV7_MSG6_P2

	SID_ECUP_CALIB_QUERY = 1792,
	SID_ECUP_CALIB1 = 1793,
	SID_ECUP_CALIB2 = 1794,

	SID_ECUS_DIFF_COEFFS = 1104,	// DEV6_MSG1_P4
};

typedef struct {
	uint8_t preambule;		// 0xAA
	uint8_t dev_uid;
	uint8_t status;
	uint8_t err_cnt;
	uint8_t hw_ver;
	uint8_t fw_ver;
	uint8_t caps;
	uint8_t reserved;
}
FSE04_StatusMsg_t;

typedef struct {
	uint8_t flags;
}
ECUA4_SDC_t;

typedef struct {
	uint8_t statusCode;
	uint8_t extendedCode;
	uint8_t cellImbalance;
}
ECUA4_AirsStatus_t;

typedef struct {
	uint8_t Uaccu[3];
	uint8_t Uair[3];
	uint8_t maxAllowedPower;
}
ECUA4_Voltages_t;

typedef struct {
	int16_t frontRight;
	int16_t frontLeft;
	int16_t rearLeft;
	int16_t rearRight;
}
ECUP4_Throttle_t;

typedef struct {
	uint8_t status;
	uint8_t brake;
	uint8_t throttle;
	uint8_t brakePressure;
	uint8_t status2;
}
ECUP4_PedalStatus_t;

typedef struct {
	uint8_t status;
}
ECUP4_SDC_t;

typedef struct {
	uint8_t frontHigh;
	uint8_t frontLow;
	uint8_t rearHigh;
	uint8_t rearLow;
	uint8_t frontMaxSlip;
	uint8_t rearMaxSlip;
	uint8_t motorEnable;
	uint8_t powerLimit;
}
ECUS_DiffCoeffs_t;

enum {
	BMS_COMMAND_CALIBRATE = 1,
};

enum {
	CAR_STATUS_NOT_READY,
	CAR_STATUS_TSON_READY,
	CAR_STATUS_PRECHARGE,
	CAR_STATUS_LATCH,
	CAR_STATUS_START_READY,
	CAR_STATUS_STARTED,
};

enum {
	ECUA4_SDC_IN = (1 << 6),
	ECUA4_SDC_IMD = (1 << 5),
	ECUA4_SDC_AMS = (1 << 4),
	ECUA4_SDC_ALL_CONNECTED = (1 << 0),
};

enum {
	ECUP4_PS_BRAKE_ACTIVE = 1,
	ECUP4_PS_CALIBRATION_OK = 2,
	ECUP4_PS_BRAKE_OK = 4,
	ECUP4_PS_T1_OK = 8,
	ECUP4_PS_T2_OK = 16,
	ECUP4_PS_BRAKE_CONN = 32,
	ECUP4_PS_T1_CONN = 64,
	ECUP4_PS_T2_CONN = 128,

	ECUP4_BSPD_ERR = 1,
	ECUP4_THROTTLE_PLAUS = 2,
	ECUP4_BRAKE_PLAUS = 4,
};

typedef enum {
	ECUA4_OK = 0,
	//ECUA4_SDC_TIMEOUT = 1,		// vyprsel timeout na uzavreni SDC
	ECUA4_HVM_MELT = 2,			// speceny AIR-
	ECUA4_HVP_MELT = 3,			// speceny AIR+
	ECUA4_HVM_TIMEOUT = 4,		// AIR- se neuzavrel
	ECUA4_HVP_TIMEOUT = 5,		// AIR+ se neuzavrel
	ECUA4_VDIFF_TIMEOUT = 6,	// napeti mezi ACCU a AIR se nesrovnalo
	ECUA4_SDC_INTERRUPTED = 7,	// preruseni SDC za behu
	ECUA4_HVM_DISCONNECT = 8,	// AIR- se otevrel za behu
	ECUA4_HVP_DISCONNECT = 9,	// AIR- se otevrel za behu
	ECUA4_BMS_CELL_MIN_VOLTAGE = 10,	// nektery clanek ma mensi napeti nez povolene minimum
	ECUA4_BMS_CELL_BALANCE = 11,		// prilis velky rozdil napeti na 2 clancich
	ECUA4_BMS_CELL_TEMPERATURE = 12,	// prilis horky clanek
	ECUA4_BMS_COMMUNICATION = 13,		// BMS nekomunikuji
	ECUA4_PRECHARGE_SLOPE = 14,
	ECUA4_TOO_MANY_ERRORS = 128,	// prilis mnoho nefatalnich chyb -> fatalni chyba
}
ECUA4_Status;

typedef enum {
	ECUC_STATE_ERROR = 0,
	ECUC_PS_NOT_CONNECTED = 1,
}
ECUC_ErrorCode;
