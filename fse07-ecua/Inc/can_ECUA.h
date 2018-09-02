#ifndef CAN_ECUA_H
#define CAN_ECUA_H

#include <tx2/can.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    bus_CAN1_powertrain = 0,
    bus_CAN2_aux = 1,
    bus_UNDEFINED = 2,
};

enum { CCU_ChargerHeartbeat_id = 0x39B };
enum { CCU_ChargerHeartbeat_timeout = 1000 };
enum { CCU_Announce_id = 0x39C };
enum { ECUA_Status_id = 0x090 };
enum { ECUA_Status_timeout = 500 };
enum { ECUA_Limits_id = 0x191 };
enum { ECUA_Limits_timeout = 500 };
enum { ECUA_ACPMeas_id = 0x392 };
enum { ECUA_ACPMeas_timeout = 100 };
enum { ECUA_Estimation_id = 0x394 };
enum { ECUA_Estimation_timeout = 500 };
enum { ECUA_BalancingStatus_id = 0x494 };
enum { ECUA_BalancingStatus2_id = 0x495 };
enum { ECUA_AMSVolts_id = 0x496 };
enum { ECUA_AMSVolts_timeout = 50 };
enum { ECUA_AMSTemp_id = 0x497 };
enum { ECUA_AMSTemp_timeout = 50 };
enum { ECUA_AMSStack_id = 0x498 };
enum { ECUA_AMSOverall_id = 0x49A };
enum { ECUA_Voltages_id = 0x395 };
enum { ECUA_Voltages_timeout = 5000 };
enum { ECUB_Status_id = 0x0A0 };
enum { ECUB_Status_timeout = 1000 };

enum ECUA_AIRsState {
    /* Awaiting detection of car or charger */
    ECUA_AIRsState_pendingChargerDetect = 0,
    /* Awaiting SDC completion to begin precharge */
    ECUA_AIRsState_ready = 1,
    /* Startup check */
    ECUA_AIRsState_startupCheck = 2,
    /* Precharge relay is being closed */
    ECUA_AIRsState_prechargeClosing = 3,
    /* HV- relay is being closed */
    ECUA_AIRsState_hvMClosing = 4,
    /* Precharge part 1: blind delay */
    ECUA_AIRsState_precharging = 5,
    /* Precharge part 2: wait for ΔV to approach zero */
    ECUA_AIRsState_waitingForVdiff = 6,
    /* HV+ relay is being closed */
    ECUA_AIRsState_hvPClosing = 7,
    /* Precharge is finishing */
    ECUA_AIRsState_endPrecharge = 8,
    /* TS is charged and active */
    ECUA_AIRsState_tsActive = 9,
    /* A fatal error occured and the AMS is latched */
    ECUA_AIRsState_fatalError = 10,
    /* Precharge failed, cooling down before retrying */
    ECUA_AIRsState_cooldown = 12,
    /* HV- failed to close, cooling down before retrying */
    ECUA_AIRsState_hvmCooldown = 13,
};

enum ECUA_StateAMS {
    /* without_fault */
    ECUA_StateAMS_All_OK = 0,
    /* All_fucked */
    ECUA_StateAMS_SHIT = 1,
};

enum ECUA_ErrorCode {
    /* no error */
    ECUA_ErrorCode_OK = 0,
    /* unused */
    ECUA_ErrorCode_SDC_TIMEOUT = 1,
    /* HV- closed even though it shouldn't be */
    ECUA_ErrorCode_HVM_MELT = 2,
    /* HV+ closed even though it shouldn't be */
    ECUA_ErrorCode_HVP_MELT = 3,
    /* HV- didn't close in time */
    ECUA_ErrorCode_HVM_TIMEOUT = 4,
    /* HV+ didn't close in time */
    ECUA_ErrorCode_HVP_TIMEOUT = 5,
    /* precharge failed to equaize AIRs x Accu voltage */
    ECUA_ErrorCode_VDIFF_TIMEOUT = 6,
    /* SDC interrupted unexpectedly */
    ECUA_ErrorCode_SDC_INTERRUPTED = 7,
    /* HV- broke open */
    ECUA_ErrorCode_HVM_DISCONNECT = 8,
    /* HV+ broke open */
    ECUA_ErrorCode_HVP_DISCONNECT = 9,
    /* Voltage of a cell is too low */
    ECUA_ErrorCode_BMS_CELL_MIN_VOLTAGE = 10,
    /* Imbalance between some cells is too high */
    ECUA_ErrorCode_BMS_CELL_BALANCE = 11,
    /* Too hot! */
    ECUA_ErrorCode_BMS_CELL_TEMPERATURE = 12,
    /* BMS communication error - probably not connected */
    ECUA_ErrorCode_BMS_COMMUNICATION = 13,
    /* Too many non-fatal errors => escalated to a fatal error */
    ECUA_ErrorCode_TOO_MANY_ERRORS = 128,
};

enum ECUB_Batt_code {
    /* No power drawn nor charged */
    ECUB_Batt_code_IDLE = 0,
    /* Charging with balancing */
    ECUB_Batt_code_CHARGING = 1,
    /* Charging without balancing */
    ECUB_Batt_code_FAST_CHARGING = 2,
    /* Only balancing */
    ECUB_Batt_code_BALANCING = 3,
    /* Is being discharged */
    ECUB_Batt_code_DISCHARGING = 4,
    /* Fully charged */
    ECUB_Batt_code_FULL = 5,
    /* Is in error state */
    ECUB_Batt_code_ERROR = 6,
};

enum ECUB_CarState {
    /* SDC interrupted -> not ready for start */
    ECUB_CarState_NOT_READY = 0,
    /* Fatal error \w SDC latching */
    ECUB_CarState_LATCHED = 1,
    /* Ready for TSON button */
    ECUB_CarState_TS_READY = 2,
    /* ACP is being precharged -> waiting for ECUA status */
    ECUB_CarState_PRECHARGE = 3,
    /* Ready for START */
    ECUB_CarState_TS_ON = 4,
    /* Waiting for completion of RTDS */
    ECUB_CarState_WAITING_FOR_RTDS = 5,
    /* Drive! */
    ECUB_CarState_STARTED = 6,
};

enum ECUB_GLV_PowerSource {
    /* ACP */
    ECUB_GLV_PowerSource_ACP = 0,
    /* GLV battery */
    ECUB_GLV_PowerSource_GLV_BATTERY = 1,
    /* Service box input */
    ECUB_GLV_PowerSource_SERVICE_INPUT = 2,
};

enum ECUB_Notready_reason {
    /* No error */
    ECUB_Notready_reason_NONE = 0,
    /* Vehicle was latched at the startup */
    ECUB_Notready_reason_LATCH_START = 1,
    /* Vehicle was latched due to BSPD error */
    ECUB_Notready_reason_LATCH_BSPD = 2,
    /* Vehicle was latched due to AMS error */
    ECUB_Notready_reason_LATCH_AMS = 3,
    /* Error in SDC chain */
    ECUB_Notready_reason_SDC_FAILURE = 4,
    /* Motor controller CAN timeout */
    ECUB_Notready_reason_TIMEOUT_MC = 5,
    /* AMS CAN timeout */
    ECUB_Notready_reason_TIMEOUT_ECUA = 6,
    /* ECU Front CAN timeout */
    ECUB_Notready_reason_TIMEOUT_ECUF = 7,
    /* Pedal unit CAN timeout */
    ECUB_Notready_reason_TIMEOUT_ECUP = 8,
    /* VDCU CAN timeout */
    ECUB_Notready_reason_TIMEOUT_VDCU = 9,
    /* Fault on PWM_fault pins */
    ECUB_Notready_reason_PWM_FAULT = 10,
};

/*
 * Hotfix for a missing interlock. Sent by ECUA to CCU.
 */
typedef struct CCU_ChargerHeartbeat_t {
	/* preambula (magic number) */
	uint8_t	HBprmb;

	/* 1 of AIRs are closed correctly, zero otherwise */
	uint8_t	ready;
} CCU_ChargerHeartbeat_t;

/*
 * Base status of ECUA and its subsystems (AMS)
 */
typedef struct ECUA_Status_t {
	/* Lead from ECUB (1 = open) */
	uint8_t	SDC_IN_Open;

	/* True if HV interlock is closed */
	uint8_t	SDC_HV_ILOCK;

	/* True if SDC is not broken by IMD */
	uint8_t	SDC_IMD;

	/* AMS = ECUA+BMS */
	uint8_t	SDC_AMS;

	/* SDC out to final stretch (BSPD etc...) */
	uint8_t	SDC_OUT;

	/* End of SDC (input to AIRS) */
	uint8_t	SDC_END;

	/* HW latch engaged (caused by IMD or AMS) */
	uint8_t	LATCH_SDC_AMS;

	/* State of AIRs */
	enum ECUA_AIRsState	AIRsState;

	/* AIRs error code */
	uint8_t	AIRs_errno;

	/* Fault of ACP overtemperature */
	uint8_t	FT_ACP_OT;

	/* Fault of AIRs */
	uint8_t	FT_AIRS;

	/* Fault of DCDC GLV converter */
	uint8_t	FT_DCDC;

	/* FAN1 dead */
	uint8_t	FT_FAN1;

	/* FAN2 dead */
	uint8_t	FT_FAN2;

	/* FAN3 dead */
	uint8_t	FT_FAN3;

	/* Fault HV overvoltage */
	uint8_t	FT_HV_OV;

	/* Fault HV undervoltage */
	uint8_t	FT_HV_UV;

	/* Fault of GLV (undervoltage measurement) */
	uint8_t	FT_GLV_UV;

	/* Fault of GLV (overvoltage measurement) */
	uint8_t	FT_GLV_OV;

	/* Internal ECUA/BMS faults */
	uint8_t	FT_AMS;

	/* If any error is present */
	uint8_t	FT_ANY;

	/* Warning for cell tempreature (near limits) */
	uint8_t	WARN_TEMP_Cell;

	/* Warning for dc-dc temperature (near limits) */
	uint8_t	WARN_TEMP_DCDC;

	/* Warning for balancer temperature (near limits) */
	uint8_t	WARN_TEMP_Bal;

	/* ACP is attached to charger? */
	uint8_t	charger_attached;

	/* 24V power output is active */
	uint8_t	DCDC_GLV_EN;

	/* Fans are active */
	uint8_t	FANS_EN;

	/* Message up counter for safety */
	uint8_t	SEQ;
} ECUA_Status_t;

/*
 * ACP power limits that are calculated in time based on temperature and votlages.
 * 
 * For example if undervoltage the PWR_OUT should be some few kW value to let the driver decide to risk.
 * If ACP is full PWR_IN should be 0.
 */
typedef struct ECUA_Limits_t {
	/* ACP output power maximum limit */
	uint16_t	PWR_OUT;

	/* ACP input power maximum limit */
	uint16_t	PWR_IN;
} ECUA_Limits_t;

/*
 * Main ACP measurements
 */
typedef struct ECUA_ACPMeas_t {
	/* HV before AIRs */
	uint16_t	Volt_HV_in;

	/* HV at output of ACP */
	uint16_t	Volt_HV_out;

	/* HV Current (positive if energy flowing outwards) */
	uint16_t	Curr_HV_out;

	/* LV Current */
	uint8_t	Curr_DCDC_out;

	/* Fans current */
	uint8_t	Curr_FANS;
} ECUA_ACPMeas_t;

/*
 * Calculated data in ACP
 */
typedef struct ECUA_Estimation_t {
	/* Charge lost in motoring */
	uint16_t	Charge_OUT;

	/* Charge reccuperated */
	uint16_t	Charge_IN;

	/* State of charge estimation */
	uint8_t	SOC;
} ECUA_Estimation_t;

/*
 * 
 */
typedef struct ECUA_BalancingStatus_t {
	/*  */
	uint8_t	active[12];

	/*  */
	uint8_t	cellIndex[12];
} ECUA_BalancingStatus_t;

/*
 * 
 */
typedef struct ECUA_BalancingStatus2_t {
	/*  */
	uint16_t	Vtarget;

	/*  */
	uint16_t	AllowedVdiff;

	/*  */
	uint16_t	TimeSinceLastDchg;
} ECUA_BalancingStatus2_t;

/*
 * AMS message for voltages. Uses multiplexors for identifications of dataset. AMS measures all stacks in the same time but sent that in specific order determined by muxing.
 */
typedef struct ECUA_AMSVolts_t {
	/* Index of stack in ACP - mx */
	uint8_t	StackID;

	/* Index of parallel cell group measurement  in stack - mx */
	uint8_t	SetID;

	/* Timestamp of cell measurements */
	uint16_t	Timestamp;

	/* Voltages of cell sets in parralel */
	uint16_t	Volt[2];
} ECUA_AMSVolts_t;

/*
 * AMS message for temperatures. Uses multiplexors for identification of dataset.
 */
typedef struct ECUA_AMSTemp_t {
	/* Index of stack in ACP - mx */
	uint8_t	StackID;

	/* Index of measurement in stack - mx */
	uint8_t	SetID;

	/* Temperature measurement */
	uint8_t	Temp[6];
} ECUA_AMSTemp_t;

/*
 * BMS status for each stack
 * 
 * Not recommended for new designs (obsolete)
 */
typedef struct ECUA_AMSStack_t {
	/* Index of stack in ACP - multiplexed */
	uint8_t	StackID;

	/* Maximum AMS stack cell voltage */
	uint16_t	Volt_Min;

	/* Minimum AMS stack cell voltage */
	uint16_t	Volt_Max;

	/* Maximum AMS stack cell temperature */
	uint8_t	Temp_Max;

	/* Minimum AMS stack cell temperature */
	uint8_t	Temp_Min;
} ECUA_AMSStack_t;

/*
 * AMS Overal data
 * 
 * Auch ein bisschen redundant
 */
typedef struct ECUA_AMSOverall_t {
	/* Maximum cell voltage over ACP */
	uint16_t	Volt_Max;

	/* Minimum cell voltage over ACP */
	uint16_t	Volt_Min;

	/* Maximum cell temperature over ACP */
	int8_t	Temp_Min;

	/* Minimum cell temperature over ACP */
	int8_t	Temp_Max;

	/* Maximum cell index */
	uint8_t	Temp_Max_Index;

	/* Minimum cell index */
	uint8_t	Volt_Min_index;
} ECUA_AMSOverall_t;

/*
 * Accumulator voltage information.
 */
typedef struct ECUA_Voltages_t {
	/* Voltage on accumulator stacks */
	uint32_t	Uin;

	/* Voltage on AIRs outut */
	uint32_t	Uout;
} ECUA_Voltages_t;

/*
 * ECUB Status report
 */
typedef struct ECUB_Status_t {
	/* Shutdown circuit - Front */
	uint8_t	SDC_FRONT;

	/* Shutdown circuit - Shutdown button left */
	uint8_t	SDC_SDBL;

	/* Shutdown circuit - Shutdown button right */
	uint8_t	SDC_SDBR;

	/* Shutdown circuit - High voltage disconnect */
	uint8_t	SDC_HVD;

	/* Shutdown circuit - Brake plausibility device */
	uint8_t	SDC_BSPD;

	/* Shutdown circuit - Motor controller rear */
	uint8_t	SDC_MCUR;

	/* Shutdown circuit - Accumulator management system */
	uint8_t	SDC_AMS;

	/* Shutdown circuit - Tractive system master switch */
	uint8_t	SDC_TSMS;

	/* Current vehicle state */
	enum ECUB_CarState	CarState;

	/* Reason for latest not-ready CarState */
	enum ECUB_Notready_reason	CarState_Notready;

	/* Current powering source */
	enum ECUB_GLV_PowerSource	PowerSource;

	/* Module 1 detected */
	uint8_t	Det_MOD1;

	/* Module 2 detected */
	uint8_t	Det_MOD2;

	/* Module 3 detected */
	uint8_t	Det_MOD3;

	/* Module 4 detected */
	uint8_t	Det_MOD4;

	/* Fault temperature shutdown for driver on PWR_ECUF_EN, PWR_ECUA_EN */
	uint8_t	FT_PWR1_OT;

	/* Fault temperature shutdown for driver on PWR_ECUMF_EN, PWR_ECUMR_EN */
	uint8_t	FT_PWR2_OT;

	/* Fault temperature shutdown for driver on WP1_EN, WP2_EN */
	uint8_t	FT_PWR3_OT;

	/* Fault temperature shutdown for driver on EM, FAN3_EN */
	uint8_t	FT_PWR4_OT;

	/* Fault temperature shutdown for driver on FAN1_EN, FAN2_EN */
	uint8_t	FT_PWR5_OT;

	/* Fault temperature shutdown for driver on RTDS, SDB LED R i L, BrakeLight */
	uint8_t	FT_L2_OT;

	/* If any error present */
	uint8_t	FT_ANY;

	/* Fault temperature shutdown for driver on WS, AUX1,2,3 */
	uint8_t	FT_L1_OT;

	/* Fault ECUF power (Short to VCC) */
	uint8_t	FT_PWR_ECUF_OC;

	/* Fault ECUA power (Short to VCC) */
	uint8_t	FT_PWR_ECUA_OC;

	/* Fault MCF power (Short to VCC) */
	uint8_t	FT_PWR_MCF_OC;

	/* Fault MCR power (Short to VCC) */
	uint8_t	FT_PWR_MCR_OC;

	/* Fault of CAN Bus (CAN errors or controller mode is error) */
	uint8_t	FT_CAN1;

	/* Fault of CAN Bus (CAN errors or controller mode is error) */
	uint8_t	FT_CAN2;

	/* Power to ECUF enabled */
	uint8_t	PWR_ECUF_EN;

	/* Power to ECUA enabled */
	uint8_t	PWR_ECUA_EN;

	/* Power to MCUF enabled */
	uint8_t	PWR_MCUF_EN;

	/* Power to MCUR enabled */
	uint8_t	PWR_MCUR_EN;

	/* Power to Energy Meter enabled */
	uint8_t	PWR_EM_EN;

	/* Power to Waterpump 1 enabled */
	uint8_t	PWR_WP1_EN;

	/* Power to Waterpump 2 enabled */
	uint8_t	PWR_WP2_EN;

	/* Power to FAN1 enabled */
	uint8_t	PWR_FAN1_EN;

	/* Power to FAN2 enabled */
	uint8_t	PWR_FAN2_EN;

	/* Power to FAN3 enabled */
	uint8_t	PWR_FAN3_EN;

	/* Power to Wheel speed sensor enabled */
	uint8_t	PWR_WS_EN;

	/* Power to aux1 enabled aka LIDK1 (Low I Don't Know #1) */
	uint8_t	PWR_AUX1_EN;

	/* Power to aux2 enabled aka LIDK2 */
	uint8_t	PWR_AUX2_EN;

	/* Power to aux3 enabled aka LIDK3 */
	uint8_t	PWR_AUX3_EN;

	/* Ready to drive sound enabled */
	uint8_t	RTDS_EN;

	/* Shutdown button right led enabled */
	uint8_t	SDBR_LED_EN;

	/* Shutdown button left led enabled */
	uint8_t	SDBL_LED_EN;

	/* Brakelight enabled */
	uint8_t	BrakeLight_EN;

	/* TSAL ""Test"" or ""Override"" totally not a hack enabled */
	uint8_t	TSAL_Override;

	/* Message up counter for safety check */
	uint8_t	SEQ;
} ECUB_Status_t;

void candbInit(void);

int CCU_send_ChargerHeartbeat_s(const CCU_ChargerHeartbeat_t* data);
int CCU_send_ChargerHeartbeat(uint8_t HBprmb, uint8_t ready);
int CCU_ChargerHeartbeat_need_to_send(void);

int CCU_get_Announce(void);
void CCU_Announce_on_receive(int (*callback)());

int ECUA_send_Status_s(const ECUA_Status_t* data);
int ECUA_send_Status(uint8_t SDC_IN_Open, uint8_t SDC_HV_ILOCK, uint8_t SDC_IMD, uint8_t SDC_AMS, uint8_t SDC_OUT, uint8_t SDC_END, uint8_t LATCH_SDC_AMS, enum ECUA_AIRsState AIRsState, uint8_t AIRs_errno, uint8_t FT_ACP_OT, uint8_t FT_AIRS, uint8_t FT_DCDC, uint8_t FT_FAN1, uint8_t FT_FAN2, uint8_t FT_FAN3, uint8_t FT_HV_OV, uint8_t FT_HV_UV, uint8_t FT_GLV_UV, uint8_t FT_GLV_OV, uint8_t FT_AMS, uint8_t FT_ANY, uint8_t WARN_TEMP_Cell, uint8_t WARN_TEMP_DCDC, uint8_t WARN_TEMP_Bal, uint8_t charger_attached, uint8_t DCDC_GLV_EN, uint8_t FANS_EN, uint8_t SEQ);
int ECUA_Status_need_to_send(void);

int ECUA_send_Limits_s(const ECUA_Limits_t* data);
int ECUA_send_Limits(uint16_t PWR_OUT, uint16_t PWR_IN);
int ECUA_Limits_need_to_send(void);

int ECUA_send_ACPMeas_s(const ECUA_ACPMeas_t* data);
int ECUA_send_ACPMeas(uint16_t Volt_HV_in, uint16_t Volt_HV_out, uint16_t Curr_HV_out, uint8_t Curr_DCDC_out, uint8_t Curr_FANS);
int ECUA_ACPMeas_need_to_send(void);

int ECUA_send_Estimation_s(const ECUA_Estimation_t* data);
int ECUA_send_Estimation(uint16_t Charge_OUT, uint16_t Charge_IN, uint8_t SOC);
int ECUA_Estimation_need_to_send(void);

int ECUA_send_BalancingStatus_s(const ECUA_BalancingStatus_t* data);
int ECUA_send_BalancingStatus(uint8_t* active, uint8_t* cellIndex);
int ECUA_BalancingStatus_need_to_send(void);

int ECUA_send_BalancingStatus2_s(const ECUA_BalancingStatus2_t* data);
int ECUA_send_BalancingStatus2(uint16_t Vtarget, uint16_t AllowedVdiff, uint16_t TimeSinceLastDchg);
int ECUA_BalancingStatus2_need_to_send(void);

int ECUA_send_AMSVolts_s(const ECUA_AMSVolts_t* data);
int ECUA_send_AMSVolts(uint8_t StackID, uint8_t SetID, uint16_t Timestamp, uint16_t* Volt);
int ECUA_AMSVolts_need_to_send(void);

int ECUA_send_AMSTemp_s(const ECUA_AMSTemp_t* data);
int ECUA_send_AMSTemp(uint8_t StackID, uint8_t SetID, uint8_t* Temp);
int ECUA_AMSTemp_need_to_send(void);

int ECUA_send_AMSStack_s(const ECUA_AMSStack_t* data);
int ECUA_send_AMSStack(uint8_t StackID, uint16_t Volt_Min, uint16_t Volt_Max, uint8_t Temp_Max, uint8_t Temp_Min);
int ECUA_AMSStack_need_to_send(void);

int ECUA_send_AMSOverall_s(const ECUA_AMSOverall_t* data);
int ECUA_send_AMSOverall(uint16_t Volt_Max, uint16_t Volt_Min, int8_t Temp_Min, int8_t Temp_Max, uint8_t Temp_Max_Index, uint8_t Volt_Min_index);
int ECUA_AMSOverall_need_to_send(void);

int ECUA_send_Voltages_s(const ECUA_Voltages_t* data);
int ECUA_send_Voltages(uint32_t Uin, uint32_t Uout);
int ECUA_Voltages_need_to_send(void);

int ECUB_decode_Status_s(const uint8_t* bytes, size_t length, ECUB_Status_t* data_out);
int ECUB_decode_Status(const uint8_t* bytes, size_t length, uint8_t* SDC_FRONT_out, uint8_t* SDC_SDBL_out, uint8_t* SDC_SDBR_out, uint8_t* SDC_HVD_out, uint8_t* SDC_BSPD_out, uint8_t* SDC_MCUR_out, uint8_t* SDC_AMS_out, uint8_t* SDC_TSMS_out, enum ECUB_CarState* CarState_out, enum ECUB_Notready_reason* CarState_Notready_out, enum ECUB_GLV_PowerSource* PowerSource_out, uint8_t* Det_MOD1_out, uint8_t* Det_MOD2_out, uint8_t* Det_MOD3_out, uint8_t* Det_MOD4_out, uint8_t* FT_PWR1_OT_out, uint8_t* FT_PWR2_OT_out, uint8_t* FT_PWR3_OT_out, uint8_t* FT_PWR4_OT_out, uint8_t* FT_PWR5_OT_out, uint8_t* FT_L2_OT_out, uint8_t* FT_ANY_out, uint8_t* FT_L1_OT_out, uint8_t* FT_PWR_ECUF_OC_out, uint8_t* FT_PWR_ECUA_OC_out, uint8_t* FT_PWR_MCF_OC_out, uint8_t* FT_PWR_MCR_OC_out, uint8_t* FT_CAN1_out, uint8_t* FT_CAN2_out, uint8_t* PWR_ECUF_EN_out, uint8_t* PWR_ECUA_EN_out, uint8_t* PWR_MCUF_EN_out, uint8_t* PWR_MCUR_EN_out, uint8_t* PWR_EM_EN_out, uint8_t* PWR_WP1_EN_out, uint8_t* PWR_WP2_EN_out, uint8_t* PWR_FAN1_EN_out, uint8_t* PWR_FAN2_EN_out, uint8_t* PWR_FAN3_EN_out, uint8_t* PWR_WS_EN_out, uint8_t* PWR_AUX1_EN_out, uint8_t* PWR_AUX2_EN_out, uint8_t* PWR_AUX3_EN_out, uint8_t* RTDS_EN_out, uint8_t* SDBR_LED_EN_out, uint8_t* SDBL_LED_EN_out, uint8_t* BrakeLight_EN_out, uint8_t* TSAL_Override_out, uint8_t* SEQ_out);
int ECUB_get_Status(ECUB_Status_t* data_out);
void ECUB_Status_on_receive(int (*callback)(ECUB_Status_t* data));

#ifdef __cplusplus
}
#endif

#endif
