

#include "timer.h"



void Init_TIM4(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 13; /* caa 5kHz */
	TIM_TimeBaseInitStruct.TIM_Period = 1000;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
	
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2; /* Pozor! Inverze v HW! */
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStruct.TIM_Pulse = 0;
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM4, &TIM_OCInitStruct);
	
	TIM4->CCR1 = 0;
	
	TIM_Cmd(TIM4, ENABLE);
}


void SetFANSpeed(uint16_t Speed)
{
  TIM4->CCR1 = Speed;
}






/* END OF FILE */
