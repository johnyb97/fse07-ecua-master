#include "can.h"

#include <can_ECUA.h>

#include <string.h>


CanRxMsg RxMessage;



void Init_CAN_Interfaces(void)
{
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
  /* CAN register init */
  CAN_DeInit(CAN1);
  CAN_StructInit(&CAN_InitStructure);
	
	/* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = ENABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = ENABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  
  /* CAN Baudrate */
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
  
	#if CAN1_BAUDRATE == 1000 /* 1MBps */
  CAN_InitStructure.CAN_Prescaler =6;
#elif CAN1_BAUDRATE == 500 /* 500KBps */
  CAN_InitStructure.CAN_Prescaler =12;
#elif CAN1_BAUDRATE == 250 /* 250KBps */
  CAN_InitStructure.CAN_Prescaler =24;
#elif CAN1_BAUDRATE == 125 /* 125KBps */
  CAN_InitStructure.CAN_Prescaler =48;
#elif  CAN1_BAUDRATE == 100 /* 100KBps */
  CAN_InitStructure.CAN_Prescaler =60;
#elif  CAN1_BAUDRATE == 50 /* 50KBps */
  CAN_InitStructure.CAN_Prescaler =120;
#elif  CAN1_BAUDRATE == 20 /* 20KBps */
  CAN_InitStructure.CAN_Prescaler =300;
#elif  CAN1_BAUDRATE == 10 /* 10KBps */
  CAN_InitStructure.CAN_Prescaler =600;
#else
   #error "Please select first the CAN1 Baudrate in can.h!"
#endif 
	
  CAN_Init(CAN1, &CAN_InitStructure);
	
	/* CAN filter init */
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);

  NVIC_InitStruct.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}


int CAN1_TransmitMessage(uint32_t StdID, const uint8_t *data, uint8_t length)
{
  CanTxMsg TxMessage;
	
	TxMessage.StdId = StdID;
  TxMessage.ExtId = 0;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = length;
	memcpy(TxMessage.Data, data, length);
  return CAN_Transmit(CAN1, &TxMessage) != CAN_TxStatus_NoMailBox;
}



void USB_LP_CAN1_RX0_IRQHandler(void)
{
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	
  if (RxMessage.IDE == CAN_Id_Standard) {
	if (txReceiveCANMessage(bus_CAN1_powertrain, RxMessage.StdId, RxMessage.Data, RxMessage.DLC) < 0) {
		// ERROR
	}
  }
}






/* END OF FILE */
