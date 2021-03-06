

#include "stm32f10x.h"
#include "gpio.h"



void Init_GPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	
	
	AIR1_GPIO_PORT->BRR = AIR1_GPIO_PIN;
	GPIO_InitStruct.GPIO_Pin = AIR1_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(AIR1_GPIO_PORT, &GPIO_InitStruct);
	
	AIR2_GPIO_PORT->BRR = AIR2_GPIO_PIN;
	GPIO_InitStruct.GPIO_Pin = AIR2_GPIO_PIN;
	GPIO_Init(AIR2_GPIO_PORT, &GPIO_InitStruct);
	
	PRECHARGE_GPIO_PORT->BRR = PRECHARGE_GPIO_PIN;
	GPIO_InitStruct.GPIO_Pin = PRECHARGE_GPIO_PIN;
	GPIO_Init(PRECHARGE_GPIO_PORT, &GPIO_InitStruct);
	
	DCDC_HVRELAY_GPIO_PORT->BRR = DCDC_HVRELAY_GPIO_PIN;
	GPIO_InitStruct.GPIO_Pin = DCDC_HVRELAY_GPIO_PIN;
	GPIO_Init(DCDC_HVRELAY_GPIO_PORT, &GPIO_InitStruct);
	
	AMS_OK_GPIO_PORT->BSRR = AMS_OK_GPIO_PIN;
	GPIO_InitStruct.GPIO_Pin = AMS_OK_GPIO_PIN;
	GPIO_Init(AMS_OK_GPIO_PORT, &GPIO_InitStruct);
	
	DCDC_FAN_DISABLE_PORT->BSRR = DCDC_FAN_DISABLE_PIN; /* Log H default! */
	GPIO_InitStruct.GPIO_Pin = DCDC_FAN_DISABLE_PIN;
	GPIO_Init(DCDC_FAN_DISABLE_PORT, &GPIO_InitStruct);
	
	DCDC_GLV_DISABLE_PORT->BSRR = DCDC_GLV_DISABLE_PIN; /* Log H default! */
	GPIO_InitStruct.GPIO_Pin = DCDC_GLV_DISABLE_PIN;
	GPIO_Init(DCDC_GLV_DISABLE_PORT, &GPIO_InitStruct);
	
	SPI1_NSS_GPIO_PORT->BSRR = SPI1_NSS_GPIO_PIN;  /* Log H default! */
	GPIO_InitStruct.GPIO_Pin = SPI1_NSS_GPIO_PIN;
	GPIO_Init(SPI1_NSS_GPIO_PORT, &GPIO_InitStruct);
	
	BMS_WKUP_GPIO_PORT->BRR = BMS_WKUP_GPIO_PIN;
	GPIO_InitStruct.GPIO_Pin = BMS_WKUP_GPIO_PIN;
	GPIO_Init(BMS_WKUP_GPIO_PORT, &GPIO_InitStruct);

	/* INPUTS floating */
	GPIO_InitStruct.GPIO_Pin = SDC_END_SENSE_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SDC_END_SENSE_GPIO_PORT, &GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Pin = IMD_OK_GPIO_PIN;
	GPIO_Init(IMD_OK_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = SDC_HVIL_GPIO_PIN;
	GPIO_Init(SDC_HVIL_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = BMS_RX_GPIO_PIN;
	GPIO_Init(BMS_RX_GPIO_PORT, &GPIO_InitStruct);
	

	/* INPUTS with PD */
	GPIO_InitStruct.GPIO_Pin = SPI1_MISO_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(SPI1_MISO_GPIO_PORT, &GPIO_InitStruct);
	
  /* INPUTS with PU */
	GPIO_InitStruct.GPIO_Pin = CAN1RX_GPIO_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(CAN1RX_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = FAN_PGOOD_GPIO_PIN;
	GPIO_Init(FAN_PGOOD_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = AIR1_OPEN_GPIO_PIN;
	GPIO_Init(AIR1_OPEN_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = AIR2_OPEN_GPIO_PIN;
	GPIO_Init(AIR2_OPEN_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = PRECHARGE_OPEN_GPIO_PIN;
	GPIO_Init(PRECHARGE_OPEN_GPIO_PORT, &GPIO_InitStruct);
	
	
	/* OUTPUTS WITH AF */
	GPIO_InitStruct.GPIO_Pin = CAN1TX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(CAN1TX_GPIO_PORT, &GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Pin = FAN_PWM_GPIO_PIN;
	GPIO_Init(FAN_PWM_GPIO_PORT, &GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Pin = SPI1_MOSI_GPIO_PIN;
	GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = SPI1_SCK_GPIO_PIN;
	GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = BMS_TX_GPIO_PIN;
	GPIO_Init(BMS_TX_GPIO_PORT, &GPIO_InitStruct);

	/* ANALOG */
	/*GPIO_InitStruct.GPIO_Pin = LV_CURRENT_GPIO_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(LV_CURRENT_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = HV_CURRENT_GPIO_PIN;
  GPIO_Init(HV_CURRENT_GPIO_PORT, &GPIO_InitStruct);
	*/
}


/* END OF FILE */
