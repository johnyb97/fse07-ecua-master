/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp
 *
 * Copyright (c) 2018 eForce FEE Prague Formula
 */


#include "airs.hpp"
#include "bms.hpp"
#include "ecuacan.hpp"
#include "ecua_options.hpp"
#include "peripherals.hpp"
#include "sdc.hpp"
#include "voltagesense.hpp"

#include <can_ECUA.h>

#include "stm32f10x.h"
#include "gpio.h"
#include "stm32f10x_usart.h"

// Global Pins
Pin SDCend(			SDC_END_SENSE_GPIO_PORT,	-1,	SDC_END_SENSE_GPIO_PIN);
Pin airMCon_n(		AIR2_OPEN_GPIO_PORT,		-1,	AIR2_OPEN_GPIO_PIN);
Pin airPCon_n(		AIR1_OPEN_GPIO_PORT,		-1,	AIR1_OPEN_GPIO_PIN);
Pin hvInterlock(	SDC_HVIL_GPIO_PORT,			-1,	SDC_HVIL_GPIO_PIN);
Pin imdOk(			IMD_OK_GPIO_PORT,			-1, IMD_OK_GPIO_PIN);
Pin acpOK(			AMS_OK_GPIO_PORT,			-1, AMS_OK_GPIO_PIN);
Pin precharge(		PRECHARGE_GPIO_PORT,		-1, PRECHARGE_GPIO_PIN);
Pin airMEn(			AIR2_GPIO_PORT,				-1,	AIR2_GPIO_PIN);
Pin airPEn(			AIR1_GPIO_PORT,				-1,	AIR1_GPIO_PIN);

Pin bmsWkup(		BMS_WKUP_GPIO_PORT,			-1, BMS_WKUP_GPIO_PIN);

/////////////////////////

VoltageSense voltages;

static Peripherals peripherals(&voltages);
SDC g_sdc(nullptr, &hvInterlock, &imdOk, &acpOK, &SDCend);
AIRs g_airs(&peripherals, &g_sdc, &voltages, &SDCend, &airMCon_n, &airPCon_n);

ECUACAN ecuaCan(&peripherals, &voltages, &g_sdc);

extern "C" void USART3_IRQHandler(void) {
	uint8_t data = USART_ReceiveData(USART3);

	BMS::i_UartReceiveData(data);
}

extern "C" void fse07_ecua_tick() {
	g_airs.i_OnTick();
}

extern "C" void fse07_ecua_main() {
	txInit();
	candbInit();
	ecuaCan.Init();

	voltages.Init();
	g_airs.Init();
	g_sdc.Init();
	peripherals.Init();

	BMS::Init(&peripherals, &g_sdc);

	debug_printf(("ECUA: Init sequence OK\r\n"));

	HAL_Delay(10);		// wait for filtered pins to stabilize

	for (;;) {
		g_sdc.Update();
		g_airs.Update();
		voltages.Update();
		peripherals.Update();

		BMS::Update();

		txProcess();
		ecuaCan.Update();
	}
}
