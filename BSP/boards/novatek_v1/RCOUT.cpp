#include "RCOUT.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_rcc.h>
#include <string.h>

#define OC 12

namespace dev_v2
{
	
RCOUT::RCOUT()
{
	memset(channel_datas, 0, sizeof(channel_datas));
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	// open B6~9 as output (TIM4 channel 1~4)
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_TIM4);
	
	// Time base configuration
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_TimeBaseStructure.TIM_Period = 2000*OC-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 84/OC-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);
	TIM_Cmd(TIM4, ENABLE);

	TIM_ARRPreloadConfig(TIM4, ENABLE);

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);


	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
}

// total channel count
int RCOUT::get_channel_count()
{
	return MAX_CHANNEL;
}

// return num channel written
// generate an error if index overrun/underrun, and won't update any channel
// return negative value to indicate an error
int RCOUT::write(const int16_t *data, int start_channel, int count)
{
	volatile uint32_t *registers[MAX_CHANNEL] =
	{
		&TIM4->CCR1,
		
		&TIM4->CCR2,		
		&TIM4->CCR4,
		
		&TIM4->CCR3,
	};
	
	if (start_channel < 0 || start_channel + count > MAX_CHANNEL)
		return -1;
	
	for(int i=0; i<count; i++)
	{
		channel_datas[i+start_channel] = data[i];
		registers[i+start_channel][0] = 2000*OC - data[i] * OC;
	}

	TIM_Cmd(TIM4, ENABLE);
	
	return 0;
}

// return num channel read
// return any possible read if index overrun
// return negative value to indicate an error
int RCOUT::read(int16_t *out, int start_channel, int max_count)
{
	if (start_channel<0 || start_channel >= MAX_CHANNEL)
		return 0;
	
	int count = MAX_CHANNEL - start_channel;
	if (count > max_count)
		count = max_count;
	
	for(int i=0; i<count; i++)
		out[i] = channel_datas[i+start_channel];
	
	return count;	
}

}