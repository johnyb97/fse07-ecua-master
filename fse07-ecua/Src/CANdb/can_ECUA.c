#include "can_ECUA.h"
#include <string.h>

int32_t CCU_ChargerHeartbeat_last_sent;
CAN_msg_status_t CCU_Announce_status;
int32_t ECUA_Status_last_sent;
int32_t ECUA_Limits_last_sent;
int32_t ECUA_ACPMeas_last_sent;
int32_t ECUA_Estimation_last_sent;
int32_t ECUA_BalancingStatus_last_sent;
int32_t ECUA_BalancingStatus2_last_sent;
int32_t ECUA_AMSVolts_last_sent;
int32_t ECUA_AMSTemp_last_sent;
int32_t ECUA_AMSStack_last_sent;
int32_t ECUA_AMSOverall_last_sent;
int32_t ECUA_Voltages_last_sent;
CAN_msg_status_t ECUB_Status_status;
ECUB_Status_t ECUB_Status_data;

void candbInit(void) {
    CCU_ChargerHeartbeat_last_sent = -1;
    canInitMsgStatus(&CCU_Announce_status, -1);
    ECUA_Status_last_sent = -1;
    ECUA_Limits_last_sent = -1;
    ECUA_ACPMeas_last_sent = -1;
    ECUA_Estimation_last_sent = -1;
    ECUA_BalancingStatus_last_sent = -1;
    ECUA_BalancingStatus2_last_sent = -1;
    ECUA_AMSVolts_last_sent = -1;
    ECUA_AMSTemp_last_sent = -1;
    ECUA_AMSStack_last_sent = -1;
    ECUA_AMSOverall_last_sent = -1;
    ECUA_Voltages_last_sent = -1;
    canInitMsgStatus(&ECUB_Status_status, 1000);
}

int CCU_send_ChargerHeartbeat_s(const CCU_ChargerHeartbeat_t* data) {
    uint8_t buffer[2];
    buffer[0] = data->HBprmb;
    buffer[1] = data->ready;
    int rc = txSendCANMessage(bus_UNDEFINED, CCU_ChargerHeartbeat_id, buffer, sizeof(buffer));

    if (rc == 0) {
        CCU_ChargerHeartbeat_last_sent = txGetTimeMillis();
    }

    return rc;
}

int CCU_send_ChargerHeartbeat(uint8_t HBprmb, uint8_t ready) {
    uint8_t buffer[2];
    buffer[0] = HBprmb;
    buffer[1] = ready;
    int rc = txSendCANMessage(bus_UNDEFINED, CCU_ChargerHeartbeat_id, buffer, sizeof(buffer));

    if (rc == 0) {
        CCU_ChargerHeartbeat_last_sent = txGetTimeMillis();
    }

    return rc;
}

int CCU_ChargerHeartbeat_need_to_send(void) {
    return (CCU_ChargerHeartbeat_last_sent == -1) || (txGetTimeMillis() >= CCU_ChargerHeartbeat_last_sent + 100);
}

int CCU_get_Announce(void) {
    if (!(CCU_Announce_status.flags & CAN_MSG_RECEIVED))
        return 0;

    int flags = CCU_Announce_status.flags;
    CCU_Announce_status.flags &= ~CAN_MSG_PENDING;
    return flags;
}

void CCU_Announce_on_receive(int (*callback)()) {
    CCU_Announce_status.on_receive = (void (*)(void)) callback;
}

int ECUA_send_Status_s(const ECUA_Status_t* data) {
    uint8_t buffer[7];
    buffer[0] = (data->SDC_IN_Open ? 1 : 0) | (data->SDC_HV_ILOCK ? 2 : 0) | (data->SDC_IMD ? 4 : 0) | (data->SDC_AMS ? 8 : 0) | (data->SDC_OUT ? 16 : 0) | (data->SDC_END ? 32 : 0) | (data->LATCH_SDC_AMS ? 128 : 0);
    buffer[1] = (data->AIRsState & 0x0F);
    buffer[2] = data->AIRs_errno;
    buffer[3] = (data->FT_ACP_OT ? 1 : 0) | (data->FT_AIRS ? 2 : 0) | (data->FT_DCDC ? 4 : 0) | (data->FT_FAN1 ? 8 : 0) | (data->FT_FAN2 ? 16 : 0) | (data->FT_FAN3 ? 32 : 0) | (data->FT_HV_OV ? 64 : 0) | (data->FT_HV_UV ? 128 : 0);
    buffer[4] = (data->FT_GLV_UV ? 1 : 0) | (data->FT_GLV_OV ? 2 : 0) | (data->FT_AMS ? 4 : 0) | (data->FT_ANY ? 8 : 0);
    buffer[5] = (data->WARN_TEMP_Cell ? 1 : 0) | (data->WARN_TEMP_DCDC ? 2 : 0) | (data->WARN_TEMP_Bal ? 4 : 0) | (data->charger_attached ? 32 : 0) | (data->DCDC_GLV_EN ? 64 : 0) | (data->FANS_EN ? 128 : 0);
    buffer[6] = ((data->SEQ & 0x0F) << 4);
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_Status_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Status_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_Status(uint8_t SDC_IN_Open, uint8_t SDC_HV_ILOCK, uint8_t SDC_IMD, uint8_t SDC_AMS, uint8_t SDC_OUT, uint8_t SDC_END, uint8_t LATCH_SDC_AMS, enum ECUA_AIRsState AIRsState, uint8_t AIRs_errno, uint8_t FT_ACP_OT, uint8_t FT_AIRS, uint8_t FT_DCDC, uint8_t FT_FAN1, uint8_t FT_FAN2, uint8_t FT_FAN3, uint8_t FT_HV_OV, uint8_t FT_HV_UV, uint8_t FT_GLV_UV, uint8_t FT_GLV_OV, uint8_t FT_AMS, uint8_t FT_ANY, uint8_t WARN_TEMP_Cell, uint8_t WARN_TEMP_DCDC, uint8_t WARN_TEMP_Bal, uint8_t charger_attached, uint8_t DCDC_GLV_EN, uint8_t FANS_EN, uint8_t SEQ) {
    uint8_t buffer[7];
    buffer[0] = (SDC_IN_Open ? 1 : 0) | (SDC_HV_ILOCK ? 2 : 0) | (SDC_IMD ? 4 : 0) | (SDC_AMS ? 8 : 0) | (SDC_OUT ? 16 : 0) | (SDC_END ? 32 : 0) | (LATCH_SDC_AMS ? 128 : 0);
    buffer[1] = (AIRsState & 0x0F);
    buffer[2] = AIRs_errno;
    buffer[3] = (FT_ACP_OT ? 1 : 0) | (FT_AIRS ? 2 : 0) | (FT_DCDC ? 4 : 0) | (FT_FAN1 ? 8 : 0) | (FT_FAN2 ? 16 : 0) | (FT_FAN3 ? 32 : 0) | (FT_HV_OV ? 64 : 0) | (FT_HV_UV ? 128 : 0);
    buffer[4] = (FT_GLV_UV ? 1 : 0) | (FT_GLV_OV ? 2 : 0) | (FT_AMS ? 4 : 0) | (FT_ANY ? 8 : 0);
    buffer[5] = (WARN_TEMP_Cell ? 1 : 0) | (WARN_TEMP_DCDC ? 2 : 0) | (WARN_TEMP_Bal ? 4 : 0) | (charger_attached ? 32 : 0) | (DCDC_GLV_EN ? 64 : 0) | (FANS_EN ? 128 : 0);
    buffer[6] = ((SEQ & 0x0F) << 4);
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_Status_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Status_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_Status_need_to_send(void) {
    return (ECUA_Status_last_sent == -1) || (txGetTimeMillis() >= ECUA_Status_last_sent + 100);
}

int ECUA_send_Limits_s(const ECUA_Limits_t* data) {
    uint8_t buffer[4];
    buffer[0] = data->PWR_OUT;
    buffer[1] = (data->PWR_OUT >> 8);
    buffer[2] = data->PWR_IN;
    buffer[3] = (data->PWR_IN >> 8);
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_Limits_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Limits_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_Limits(uint16_t PWR_OUT, uint16_t PWR_IN) {
    uint8_t buffer[4];
    buffer[0] = PWR_OUT;
    buffer[1] = (PWR_OUT >> 8);
    buffer[2] = PWR_IN;
    buffer[3] = (PWR_IN >> 8);
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_Limits_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Limits_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_Limits_need_to_send(void) {
    return (ECUA_Limits_last_sent == -1) || (txGetTimeMillis() >= ECUA_Limits_last_sent + 100);
}

int ECUA_send_ACPMeas_s(const ECUA_ACPMeas_t* data) {
    uint8_t buffer[8];
    buffer[0] = data->Volt_HV_in;
    buffer[1] = (data->Volt_HV_in >> 8);
    buffer[2] = data->Volt_HV_out;
    buffer[3] = (data->Volt_HV_out >> 8);
    buffer[4] = data->Curr_HV_out;
    buffer[5] = (data->Curr_HV_out >> 8);
    buffer[6] = data->Curr_DCDC_out;
    buffer[7] = data->Curr_FANS;
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_ACPMeas_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_ACPMeas_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_ACPMeas(uint16_t Volt_HV_in, uint16_t Volt_HV_out, uint16_t Curr_HV_out, uint8_t Curr_DCDC_out, uint8_t Curr_FANS) {
    uint8_t buffer[8];
    buffer[0] = Volt_HV_in;
    buffer[1] = (Volt_HV_in >> 8);
    buffer[2] = Volt_HV_out;
    buffer[3] = (Volt_HV_out >> 8);
    buffer[4] = Curr_HV_out;
    buffer[5] = (Curr_HV_out >> 8);
    buffer[6] = Curr_DCDC_out;
    buffer[7] = Curr_FANS;
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_ACPMeas_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_ACPMeas_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_ACPMeas_need_to_send(void) {
    return (ECUA_ACPMeas_last_sent == -1) || (txGetTimeMillis() >= ECUA_ACPMeas_last_sent + 10);
}

int ECUA_send_Estimation_s(const ECUA_Estimation_t* data) {
    uint8_t buffer[5];
    buffer[0] = data->Charge_OUT;
    buffer[1] = (data->Charge_OUT >> 8);
    buffer[2] = data->Charge_IN;
    buffer[3] = (data->Charge_IN >> 8);
    buffer[4] = data->SOC;
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_Estimation_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Estimation_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_Estimation(uint16_t Charge_OUT, uint16_t Charge_IN, uint8_t SOC) {
    uint8_t buffer[5];
    buffer[0] = Charge_OUT;
    buffer[1] = (Charge_OUT >> 8);
    buffer[2] = Charge_IN;
    buffer[3] = (Charge_IN >> 8);
    buffer[4] = SOC;
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_Estimation_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Estimation_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_Estimation_need_to_send(void) {
    return (ECUA_Estimation_last_sent == -1) || (txGetTimeMillis() >= ECUA_Estimation_last_sent + 100);
}

int ECUA_send_BalancingStatus_s(const ECUA_BalancingStatus_t* data) {
    uint8_t buffer[8];
    buffer[0] = (data->active[0] ? 1 : 0) | (data->active[1] ? 2 : 0) | (data->active[2] ? 4 : 0) | (data->active[3] ? 8 : 0) | (data->active[4] ? 16 : 0) | (data->active[5] ? 32 : 0) | (data->active[6] ? 64 : 0) | (data->active[7] ? 128 : 0);
    buffer[1] = (data->active[8] ? 1 : 0) | (data->active[9] ? 2 : 0) | (data->active[10] ? 4 : 0) | (data->active[11] ? 8 : 0);
    buffer[2] = (data->cellIndex[0] & 0x0F) | ((data->cellIndex[1] & 0x0F) << 4);
    buffer[3] = (data->cellIndex[2] & 0x0F) | ((data->cellIndex[3] & 0x0F) << 4);
    buffer[4] = (data->cellIndex[4] & 0x0F) | ((data->cellIndex[5] & 0x0F) << 4);
    buffer[5] = (data->cellIndex[6] & 0x0F) | ((data->cellIndex[7] & 0x0F) << 4);
    buffer[6] = (data->cellIndex[8] & 0x0F) | ((data->cellIndex[9] & 0x0F) << 4);
    buffer[7] = (data->cellIndex[10] & 0x0F) | ((data->cellIndex[11] & 0x0F) << 4);
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_BalancingStatus_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_BalancingStatus_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_BalancingStatus(uint8_t* active, uint8_t* cellIndex) {
    uint8_t buffer[8];
    buffer[0] = (active[0] ? 1 : 0) | (active[1] ? 2 : 0) | (active[2] ? 4 : 0) | (active[3] ? 8 : 0) | (active[4] ? 16 : 0) | (active[5] ? 32 : 0) | (active[6] ? 64 : 0) | (active[7] ? 128 : 0);
    buffer[1] = (active[8] ? 1 : 0) | (active[9] ? 2 : 0) | (active[10] ? 4 : 0) | (active[11] ? 8 : 0);
    buffer[2] = (cellIndex[0] & 0x0F) | ((cellIndex[1] & 0x0F) << 4);
    buffer[3] = (cellIndex[2] & 0x0F) | ((cellIndex[3] & 0x0F) << 4);
    buffer[4] = (cellIndex[4] & 0x0F) | ((cellIndex[5] & 0x0F) << 4);
    buffer[5] = (cellIndex[6] & 0x0F) | ((cellIndex[7] & 0x0F) << 4);
    buffer[6] = (cellIndex[8] & 0x0F) | ((cellIndex[9] & 0x0F) << 4);
    buffer[7] = (cellIndex[10] & 0x0F) | ((cellIndex[11] & 0x0F) << 4);
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_BalancingStatus_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_BalancingStatus_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_BalancingStatus_need_to_send(void) {
    return (ECUA_BalancingStatus_last_sent == -1) || (txGetTimeMillis() >= ECUA_BalancingStatus_last_sent + 1000);
}

int ECUA_send_BalancingStatus2_s(const ECUA_BalancingStatus2_t* data) {
    uint8_t buffer[6];
    buffer[0] = data->Vtarget;
    buffer[1] = (data->Vtarget >> 8);
    buffer[2] = data->AllowedVdiff;
    buffer[3] = (data->AllowedVdiff >> 8);
    buffer[4] = data->TimeSinceLastDchg;
    buffer[5] = (data->TimeSinceLastDchg >> 8);
    int rc = txSendCANMessage(bus_UNDEFINED, ECUA_BalancingStatus2_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_BalancingStatus2_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_BalancingStatus2(uint16_t Vtarget, uint16_t AllowedVdiff, uint16_t TimeSinceLastDchg) {
    uint8_t buffer[6];
    buffer[0] = Vtarget;
    buffer[1] = (Vtarget >> 8);
    buffer[2] = AllowedVdiff;
    buffer[3] = (AllowedVdiff >> 8);
    buffer[4] = TimeSinceLastDchg;
    buffer[5] = (TimeSinceLastDchg >> 8);
    int rc = txSendCANMessage(bus_UNDEFINED, ECUA_BalancingStatus2_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_BalancingStatus2_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_BalancingStatus2_need_to_send(void) {
    return (ECUA_BalancingStatus2_last_sent == -1) || (txGetTimeMillis() >= ECUA_BalancingStatus2_last_sent + 1000);
}

int ECUA_send_AMSVolts_s(const ECUA_AMSVolts_t* data) {
    uint8_t buffer[7];
    buffer[0] = (data->StackID & 0x0F) | ((data->SetID & 0x0F) << 4);
    buffer[1] = data->Timestamp;
    buffer[2] = (data->Timestamp >> 8);
    buffer[3] = data->Volt[0];
    buffer[4] = (data->Volt[0] >> 8);
    buffer[5] = data->Volt[1];
    buffer[6] = (data->Volt[1] >> 8);
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_AMSVolts_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSVolts_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_AMSVolts(uint8_t StackID, uint8_t SetID, uint16_t Timestamp, uint16_t* Volt) {
    uint8_t buffer[7];
    buffer[0] = (StackID & 0x0F) | ((SetID & 0x0F) << 4);
    buffer[1] = Timestamp;
    buffer[2] = (Timestamp >> 8);
    buffer[3] = Volt[0];
    buffer[4] = (Volt[0] >> 8);
    buffer[5] = Volt[1];
    buffer[6] = (Volt[1] >> 8);
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_AMSVolts_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSVolts_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_AMSVolts_need_to_send(void) {
    return (ECUA_AMSVolts_last_sent == -1) || (txGetTimeMillis() >= ECUA_AMSVolts_last_sent + 100);
}

int ECUA_send_AMSTemp_s(const ECUA_AMSTemp_t* data) {
    uint8_t buffer[7];
    buffer[0] = (data->StackID & 0x0F) | ((data->SetID & 0x0F) << 4);
    buffer[1] = data->Temp[0];
    buffer[2] = data->Temp[1];
    buffer[3] = data->Temp[2];
    buffer[4] = data->Temp[3];
    buffer[5] = data->Temp[4];
    buffer[6] = data->Temp[5];
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_AMSTemp_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSTemp_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_AMSTemp(uint8_t StackID, uint8_t SetID, uint8_t* Temp) {
    uint8_t buffer[7];
    buffer[0] = (StackID & 0x0F) | ((SetID & 0x0F) << 4);
    buffer[1] = Temp[0];
    buffer[2] = Temp[1];
    buffer[3] = Temp[2];
    buffer[4] = Temp[3];
    buffer[5] = Temp[4];
    buffer[6] = Temp[5];
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_AMSTemp_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSTemp_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_AMSTemp_need_to_send(void) {
    return (ECUA_AMSTemp_last_sent == -1) || (txGetTimeMillis() >= ECUA_AMSTemp_last_sent + 72);
}

int ECUA_send_AMSStack_s(const ECUA_AMSStack_t* data) {
    uint8_t buffer[7];
    buffer[0] = (data->StackID & 0x0F);
    buffer[1] = data->Volt_Min;
    buffer[2] = (data->Volt_Min >> 8);
    buffer[3] = data->Volt_Max;
    buffer[4] = (data->Volt_Max >> 8);
    buffer[5] = data->Temp_Max;
    buffer[6] = data->Temp_Min;
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_AMSStack_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSStack_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_AMSStack(uint8_t StackID, uint16_t Volt_Min, uint16_t Volt_Max, uint8_t Temp_Max, uint8_t Temp_Min) {
    uint8_t buffer[7];
    buffer[0] = (StackID & 0x0F);
    buffer[1] = Volt_Min;
    buffer[2] = (Volt_Min >> 8);
    buffer[3] = Volt_Max;
    buffer[4] = (Volt_Max >> 8);
    buffer[5] = Temp_Max;
    buffer[6] = Temp_Min;
    int rc = txSendCANMessage(bus_CAN2_aux, ECUA_AMSStack_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSStack_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_AMSStack_need_to_send(void) {
    return (ECUA_AMSStack_last_sent == -1) || (txGetTimeMillis() >= ECUA_AMSStack_last_sent + 100);
}

int ECUA_send_AMSOverall_s(const ECUA_AMSOverall_t* data) {
    uint8_t buffer[8];
    buffer[0] = data->Volt_Max;
    buffer[1] = (data->Volt_Max >> 8);
    buffer[2] = data->Volt_Min;
    buffer[3] = (data->Volt_Min >> 8);
    buffer[4] = data->Temp_Min;
    buffer[5] = data->Temp_Max;
    buffer[6] = data->Temp_Max_Index;
    buffer[7] = data->Volt_Min_index;
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_AMSOverall_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSOverall_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_AMSOverall(uint16_t Volt_Max, uint16_t Volt_Min, int8_t Temp_Min, int8_t Temp_Max, uint8_t Temp_Max_Index, uint8_t Volt_Min_index) {
    uint8_t buffer[8];
    buffer[0] = Volt_Max;
    buffer[1] = (Volt_Max >> 8);
    buffer[2] = Volt_Min;
    buffer[3] = (Volt_Min >> 8);
    buffer[4] = Temp_Min;
    buffer[5] = Temp_Max;
    buffer[6] = Temp_Max_Index;
    buffer[7] = Volt_Min_index;
    int rc = txSendCANMessage(bus_CAN1_powertrain, ECUA_AMSOverall_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_AMSOverall_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_AMSOverall_need_to_send(void) {
    return (ECUA_AMSOverall_last_sent == -1) || (txGetTimeMillis() >= ECUA_AMSOverall_last_sent + 200);
}

int ECUA_send_Voltages_s(const ECUA_Voltages_t* data) {
    uint8_t buffer[7];
    buffer[0] = data->Uin;
    buffer[1] = (data->Uin >> 8);
    buffer[2] = (data->Uin >> 16);
    buffer[3] = data->Uout;
    buffer[4] = (data->Uout >> 8);
    buffer[5] = (data->Uout >> 16);
    buffer[6] = 0;
    int rc = txSendCANMessage(bus_UNDEFINED, ECUA_Voltages_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Voltages_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_send_Voltages(uint32_t Uin, uint32_t Uout) {
    uint8_t buffer[7];
    buffer[0] = Uin;
    buffer[1] = (Uin >> 8);
    buffer[2] = (Uin >> 16);
    buffer[3] = Uout;
    buffer[4] = (Uout >> 8);
    buffer[5] = (Uout >> 16);
    buffer[6] = 0;
    int rc = txSendCANMessage(bus_UNDEFINED, ECUA_Voltages_id, buffer, sizeof(buffer));

    if (rc == 0) {
        ECUA_Voltages_last_sent = txGetTimeMillis();
    }

    return rc;
}

int ECUA_Voltages_need_to_send(void) {
    return (ECUA_Voltages_last_sent == -1) || (txGetTimeMillis() >= ECUA_Voltages_last_sent + 811);
}

int ECUB_decode_Status_s(const uint8_t* bytes, size_t length, ECUB_Status_t* data_out) {
    if (length < 8)
        return 0;

    data_out->SDC_FRONT = (bytes[0] & 0x01);
    data_out->SDC_SDBL = ((bytes[0] >> 1) & 0x01);
    data_out->SDC_SDBR = ((bytes[0] >> 2) & 0x01);
    data_out->SDC_HVD = ((bytes[0] >> 3) & 0x01);
    data_out->SDC_BSPD = ((bytes[0] >> 4) & 0x01);
    data_out->SDC_MCUR = ((bytes[0] >> 5) & 0x01);
    data_out->SDC_AMS = ((bytes[0] >> 6) & 0x01);
    data_out->SDC_TSMS = ((bytes[0] >> 7) & 0x01);
    data_out->CarState = (enum ECUB_CarState) ((bytes[1] & 0x0F));
    data_out->CarState_Notready = (enum ECUB_Notready_reason) (((bytes[1] >> 4) & 0x0F));
    data_out->PowerSource = (enum ECUB_GLV_PowerSource) ((bytes[2] & 0x03));
    data_out->Det_MOD1 = ((bytes[2] >> 4) & 0x01);
    data_out->Det_MOD2 = ((bytes[2] >> 5) & 0x01);
    data_out->Det_MOD3 = ((bytes[2] >> 6) & 0x01);
    data_out->Det_MOD4 = ((bytes[2] >> 7) & 0x01);
    data_out->FT_PWR1_OT = (bytes[3] & 0x01);
    data_out->FT_PWR2_OT = ((bytes[3] >> 1) & 0x01);
    data_out->FT_PWR3_OT = ((bytes[3] >> 2) & 0x01);
    data_out->FT_PWR4_OT = ((bytes[3] >> 3) & 0x01);
    data_out->FT_PWR5_OT = ((bytes[3] >> 4) & 0x01);
    data_out->FT_L2_OT = ((bytes[3] >> 5) & 0x01);
    data_out->FT_ANY = ((bytes[3] >> 6) & 0x01);
    data_out->FT_L1_OT = ((bytes[3] >> 7) & 0x01);
    data_out->FT_PWR_ECUF_OC = (bytes[4] & 0x01);
    data_out->FT_PWR_ECUA_OC = ((bytes[4] >> 1) & 0x01);
    data_out->FT_PWR_MCF_OC = ((bytes[4] >> 2) & 0x01);
    data_out->FT_PWR_MCR_OC = ((bytes[4] >> 3) & 0x01);
    data_out->FT_CAN1 = ((bytes[4] >> 4) & 0x01);
    data_out->FT_CAN2 = ((bytes[4] >> 5) & 0x01);
    data_out->PWR_ECUF_EN = (bytes[5] & 0x01);
    data_out->PWR_ECUA_EN = ((bytes[5] >> 1) & 0x01);
    data_out->PWR_MCUF_EN = ((bytes[5] >> 2) & 0x01);
    data_out->PWR_MCUR_EN = ((bytes[5] >> 3) & 0x01);
    data_out->PWR_EM_EN = ((bytes[5] >> 4) & 0x01);
    data_out->PWR_WP1_EN = ((bytes[5] >> 5) & 0x01);
    data_out->PWR_WP2_EN = ((bytes[5] >> 6) & 0x01);
    data_out->PWR_FAN1_EN = ((bytes[5] >> 7) & 0x01);
    data_out->PWR_FAN2_EN = (bytes[6] & 0x01);
    data_out->PWR_FAN3_EN = ((bytes[6] >> 1) & 0x01);
    data_out->PWR_WS_EN = ((bytes[6] >> 2) & 0x01);
    data_out->PWR_AUX1_EN = ((bytes[6] >> 3) & 0x01);
    data_out->PWR_AUX2_EN = ((bytes[6] >> 4) & 0x01);
    data_out->PWR_AUX3_EN = ((bytes[6] >> 5) & 0x01);
    data_out->RTDS_EN = ((bytes[6] >> 6) & 0x01);
    data_out->SDBR_LED_EN = ((bytes[6] >> 7) & 0x01);
    data_out->SDBL_LED_EN = (bytes[7] & 0x01);
    data_out->BrakeLight_EN = ((bytes[7] >> 1) & 0x01);
    data_out->TSAL_Override = ((bytes[7] >> 2) & 0x01);
    data_out->SEQ = ((bytes[7] >> 4) & 0x0F);
    return 1;
}

int ECUB_decode_Status(const uint8_t* bytes, size_t length, uint8_t* SDC_FRONT_out, uint8_t* SDC_SDBL_out, uint8_t* SDC_SDBR_out, uint8_t* SDC_HVD_out, uint8_t* SDC_BSPD_out, uint8_t* SDC_MCUR_out, uint8_t* SDC_AMS_out, uint8_t* SDC_TSMS_out, enum ECUB_CarState* CarState_out, enum ECUB_Notready_reason* CarState_Notready_out, enum ECUB_GLV_PowerSource* PowerSource_out, uint8_t* Det_MOD1_out, uint8_t* Det_MOD2_out, uint8_t* Det_MOD3_out, uint8_t* Det_MOD4_out, uint8_t* FT_PWR1_OT_out, uint8_t* FT_PWR2_OT_out, uint8_t* FT_PWR3_OT_out, uint8_t* FT_PWR4_OT_out, uint8_t* FT_PWR5_OT_out, uint8_t* FT_L2_OT_out, uint8_t* FT_ANY_out, uint8_t* FT_L1_OT_out, uint8_t* FT_PWR_ECUF_OC_out, uint8_t* FT_PWR_ECUA_OC_out, uint8_t* FT_PWR_MCF_OC_out, uint8_t* FT_PWR_MCR_OC_out, uint8_t* FT_CAN1_out, uint8_t* FT_CAN2_out, uint8_t* PWR_ECUF_EN_out, uint8_t* PWR_ECUA_EN_out, uint8_t* PWR_MCUF_EN_out, uint8_t* PWR_MCUR_EN_out, uint8_t* PWR_EM_EN_out, uint8_t* PWR_WP1_EN_out, uint8_t* PWR_WP2_EN_out, uint8_t* PWR_FAN1_EN_out, uint8_t* PWR_FAN2_EN_out, uint8_t* PWR_FAN3_EN_out, uint8_t* PWR_WS_EN_out, uint8_t* PWR_AUX1_EN_out, uint8_t* PWR_AUX2_EN_out, uint8_t* PWR_AUX3_EN_out, uint8_t* RTDS_EN_out, uint8_t* SDBR_LED_EN_out, uint8_t* SDBL_LED_EN_out, uint8_t* BrakeLight_EN_out, uint8_t* TSAL_Override_out, uint8_t* SEQ_out) {
    if (length < 8)
        return 0;

    *SDC_FRONT_out = (bytes[0] & 0x01);
    *SDC_SDBL_out = ((bytes[0] >> 1) & 0x01);
    *SDC_SDBR_out = ((bytes[0] >> 2) & 0x01);
    *SDC_HVD_out = ((bytes[0] >> 3) & 0x01);
    *SDC_BSPD_out = ((bytes[0] >> 4) & 0x01);
    *SDC_MCUR_out = ((bytes[0] >> 5) & 0x01);
    *SDC_AMS_out = ((bytes[0] >> 6) & 0x01);
    *SDC_TSMS_out = ((bytes[0] >> 7) & 0x01);
    *CarState_out = (enum ECUB_CarState) ((bytes[1] & 0x0F));
    *CarState_Notready_out = (enum ECUB_Notready_reason) (((bytes[1] >> 4) & 0x0F));
    *PowerSource_out = (enum ECUB_GLV_PowerSource) ((bytes[2] & 0x03));
    *Det_MOD1_out = ((bytes[2] >> 4) & 0x01);
    *Det_MOD2_out = ((bytes[2] >> 5) & 0x01);
    *Det_MOD3_out = ((bytes[2] >> 6) & 0x01);
    *Det_MOD4_out = ((bytes[2] >> 7) & 0x01);
    *FT_PWR1_OT_out = (bytes[3] & 0x01);
    *FT_PWR2_OT_out = ((bytes[3] >> 1) & 0x01);
    *FT_PWR3_OT_out = ((bytes[3] >> 2) & 0x01);
    *FT_PWR4_OT_out = ((bytes[3] >> 3) & 0x01);
    *FT_PWR5_OT_out = ((bytes[3] >> 4) & 0x01);
    *FT_L2_OT_out = ((bytes[3] >> 5) & 0x01);
    *FT_ANY_out = ((bytes[3] >> 6) & 0x01);
    *FT_L1_OT_out = ((bytes[3] >> 7) & 0x01);
    *FT_PWR_ECUF_OC_out = (bytes[4] & 0x01);
    *FT_PWR_ECUA_OC_out = ((bytes[4] >> 1) & 0x01);
    *FT_PWR_MCF_OC_out = ((bytes[4] >> 2) & 0x01);
    *FT_PWR_MCR_OC_out = ((bytes[4] >> 3) & 0x01);
    *FT_CAN1_out = ((bytes[4] >> 4) & 0x01);
    *FT_CAN2_out = ((bytes[4] >> 5) & 0x01);
    *PWR_ECUF_EN_out = (bytes[5] & 0x01);
    *PWR_ECUA_EN_out = ((bytes[5] >> 1) & 0x01);
    *PWR_MCUF_EN_out = ((bytes[5] >> 2) & 0x01);
    *PWR_MCUR_EN_out = ((bytes[5] >> 3) & 0x01);
    *PWR_EM_EN_out = ((bytes[5] >> 4) & 0x01);
    *PWR_WP1_EN_out = ((bytes[5] >> 5) & 0x01);
    *PWR_WP2_EN_out = ((bytes[5] >> 6) & 0x01);
    *PWR_FAN1_EN_out = ((bytes[5] >> 7) & 0x01);
    *PWR_FAN2_EN_out = (bytes[6] & 0x01);
    *PWR_FAN3_EN_out = ((bytes[6] >> 1) & 0x01);
    *PWR_WS_EN_out = ((bytes[6] >> 2) & 0x01);
    *PWR_AUX1_EN_out = ((bytes[6] >> 3) & 0x01);
    *PWR_AUX2_EN_out = ((bytes[6] >> 4) & 0x01);
    *PWR_AUX3_EN_out = ((bytes[6] >> 5) & 0x01);
    *RTDS_EN_out = ((bytes[6] >> 6) & 0x01);
    *SDBR_LED_EN_out = ((bytes[6] >> 7) & 0x01);
    *SDBL_LED_EN_out = (bytes[7] & 0x01);
    *BrakeLight_EN_out = ((bytes[7] >> 1) & 0x01);
    *TSAL_Override_out = ((bytes[7] >> 2) & 0x01);
    *SEQ_out = ((bytes[7] >> 4) & 0x0F);
    return 1;
}

int ECUB_get_Status(ECUB_Status_t* data_out) {
    if (!(ECUB_Status_status.flags & CAN_MSG_RECEIVED))
        return 0;

#ifndef CANDB_IGNORE_TIMEOUTS
    if (txGetTimeMillis() > ECUB_Status_status.timestamp + ECUB_Status_timeout)
        return 0;
#endif

    if (data_out)
        memcpy(data_out, &ECUB_Status_data, sizeof(ECUB_Status_t));

    int flags = ECUB_Status_status.flags;
    ECUB_Status_status.flags &= ~CAN_MSG_PENDING;
    return flags;
}

void ECUB_Status_on_receive(int (*callback)(ECUB_Status_t* data)) {
    ECUB_Status_status.on_receive = (void (*)(void)) callback;
}

void candbHandleMessage(uint32_t timestamp, int bus, CAN_ID_t id, const uint8_t* payload, size_t payload_length) {
    switch (id) {
    case CCU_Announce_id: {
        canUpdateMsgStatusOnReceive(&CCU_Announce_status, timestamp);

        if (CCU_Announce_status.on_receive)
            ((int (*)(void)) CCU_Announce_status.on_receive)();

        break;
    }
    case ECUB_Status_id: {
        if (!ECUB_decode_Status_s(payload, payload_length, &ECUB_Status_data))
            break;

        canUpdateMsgStatusOnReceive(&ECUB_Status_status, timestamp);

        if (ECUB_Status_status.on_receive)
            ((int (*)(ECUB_Status_t*)) ECUB_Status_status.on_receive)(&ECUB_Status_data);

        break;
    }
    }
}
