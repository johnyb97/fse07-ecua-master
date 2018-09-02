

/* Includes -------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "gpio.h"
#include "can.h"
#include "timer.h"
#include "mcp3302.h"


/* Macros ---------------------------------------------------------------------*/

//#define BIG_ENDIAN_16(x)     (uint16_t)(((x)>>8)|((x)<<8)) 


/* Private typedef ------------------------------------------------------------*/
typedef struct {
	uint8_t Byte0;
	uint8_t Byte1;
	uint8_t Byte2;
	uint8_t Byte3;
} Msg_Test_t;

/* Private variables -	-------------------------------------------------------*/
__IO uint32_t SystemTicks = 0;	

Msg_Test_t Msg = {0x00, 0x11, 0x22, 0x33};


/* Private fucntion prototypes-------------------------------------------------*/
void Delay_ms(uint32_t ms);

void SystemInit(void)
{
  /* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);
	while(!(RCC_GetFlagStatus(RCC_FLAG_HSERDY))) {}
	
  /* Config FLASH interface and CLK dividers */		
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	FLASH_SetLatency(FLASH_ACR_LATENCY_2);
		
	/* Configure and enable PLL */
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
	RCC_PLLCmd(ENABLE);
	while(!(RCC_GetFlagStatus(RCC_FLAG_PLLRDY))) {}
	
  /* Switch system clock to PLL */		
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource() != RCC_CFGR_SWS_1) {}		
		
  /* Configure ADC clocks */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
}

void Init_BMS_UART(void) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitTypeDef init;
	init.USART_BaudRate = 250000;
	init.USART_WordLength = USART_WordLength_8b;
	init.USART_StopBits = USART_StopBits_1;
	init.USART_Parity = USART_Parity_No;
	init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART3, &init);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART3, ENABLE);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = USART3_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&nvic);
}

__IO uint16_t HV_in = 0;
__IO uint16_t HV_out = 0;

void fse07_ecua_main();
void fse07_ecua_tick();

int main(void)
{
  /* Configure SysTick to 1ms timebase */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config(72000);
	
	
	Init_TIM4();
	
  Init_GPIO();

  	Init_BMS_UART();
	
	Init_CAN_Interfaces();

	MCP3302_Init();
	
	AMS_Set_Fail();
	Delay_ms(1000);
	AMS_Set_OK();

	/* Vzorova sekvence sepnuti DCDC (CINCON aka postaru GAIA) */
	/* 1) oba disejbly uz davno musi byt v 1 (od inicializace GPIO) */
	/* 2) sepneme DCDC HV rele */
	//DCDC_HVRELAY_On();
	/* 3) Chvili pockame, baj vocko 500ms (nez softstart nabije kondy) */
	//Delay_ms(5000);
	/* 4) zenejblujem zdroj/e (vypnutim disejblu - ano CINCON je tajwanec rektalne stupidni) */
	//DCDC_FAN_Enable();
	//DCDC_GLV_Enable();
	/* 5) Profit. (tj krmit stovky watu generatoru hluku s vedlejsim ucinkem pruvanu) */
	
	//Delay_ms(2000);
	/* A zapnem si generator hluku (0-1000, po desetine %) */
	//SetFANSpeed(250); /* 25% je aj tak dost, nestrkat paprce do vetraku!! */
	
	
	while (0) {
		
		/* Pokusne mereni HV (vypada to ze to meri) */
		Delay_ms(100);
		HV_out = Measure_HV_Out();
		HV_in = Measure_HV_In();
			
	}

	fse07_ecua_main();

}


void Delay_ms(uint32_t ms)
{
  uint32_t tmp;
	tmp = SystemTicks + ms;
	while (SystemTicks < tmp) {}
}

void bmsDelayMsCb(int milliseconds) {
	Delay_ms(milliseconds);
}

void bmsTransmitCb(uint8_t data) {
	while (!USART_GetFlagStatus(USART3, USART_FLAG_TXE)) {
	}

	USART_SendData(USART3, data);
}

void SysTick_Handler(void)
{
	SystemTicks++;

	fse07_ecua_tick();
}

void HardFault_Handler(void)
{
	for (;;) {
	}
}

/* END OF FILE */

