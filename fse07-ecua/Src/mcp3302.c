


#include "stm32f10x.h"
#include "mcp3302.h"
#include "gpio.h"

void Init_SPI1(void);
uint32_t SPI1_Transceive(uint32_t Tx);

uint16_t Measure_HV_In(void)
{
  uint32_t tmp;
	
	/* viz datasheet section  6.3: Using the MCP3302 with Microcontroller (MCU) SPI Port */
	tmp = SPI1_Transceive(MCP3302_SINGLE | MCP3302_CHANNEL_1); 

	return tmp & 0x0FFF;
	/* Vysledek je primo v mV na vstupu ADC (neb je ADC 12b a ref je 4.096V) 
	   Pro ziskani napeti na HV konektoru prenasobit pomerem vstupniho delice, 
	   tj (10M+100k)/100k tedy vynasobit mV z ADC krat 101.
	   Pozn.: Presnost bez kalibrace lepsi nez 2%. Vhodne odecist offset asi 45mV na ADC.
	          (Offset lze zjistit snadno merenim bez pripojeneho napeti.)
	*/
}

uint16_t Measure_HV_Out(void)
{
  uint32_t tmp;
	
	tmp = SPI1_Transceive(MCP3302_SINGLE | MCP3302_CHANNEL_0);
	
	return tmp & 0x0FFF; 
	/* Vysledek je primo v mV na vstupu ADC (neb je ADC 12b a ref je 4.096V) 
	   Pro ziskani napeti na HV konektoru prenasobit pomerem vstupniho delice, 
	   tj (10M+100k)/100k tedy vynasobit mV z ADC krat 101.
	   Pozn.: Presnost bez kalibrace lepsi nez 2%. Vhodne odecist offset asi 45mV na ADC.
	          (Offset lze zjistit snadno merenim bez pripojeneho napeti.)
	*/
}


void MCP3302_Init(void)
{
  Init_SPI1();
	
	SPI_Cmd(SPI1, ENABLE);
}
	

void Init_SPI1(void)
{
  SPI_InitTypeDef SPI_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CRCPolynomial = 0;
	SPI_Init(SPI1, &SPI_InitStruct);		
	
	SPI_Cmd(SPI1, ENABLE);
}


uint32_t SPI1_Transceive(uint32_t Tx)
{
  uint32_t tmp=0;
	
	/* Flush the receive FIFO */
	while (SPI1->SR & SPI_SR_RXNE) {
	  SPI1->DR;
	}
	
	SPI1_NSS_Low();
	__NOP();
	__NOP();
	
	while (!(SPI1->SR & SPI_SR_TXE)) {}
	SPI1->DR = ((uint8_t*)&Tx)[2];
	while (!(SPI1->SR & SPI_SR_RXNE)) {}
	SPI1->DR;
		
	while (!(SPI1->SR & SPI_SR_TXE)) {}
	SPI1->DR = ((uint8_t*)&Tx)[1];
	while (!(SPI1->SR & SPI_SR_RXNE)) {}
	tmp = (SPI1->DR & 0x0F) << 8;
		
	while (!(SPI1->SR & SPI_SR_TXE)) {}
	SPI1->DR = ((uint8_t*)&Tx)[0];
	while (!(SPI1->SR & SPI_SR_RXNE)) {}
	tmp |= (uint8_t)(SPI1->DR);
		
	__NOP();
	__NOP();
  SPI1_NSS_High();
	
	return tmp;
}



/* END OF FILE */
