#include "F1Timer.h"
#include "F1SysTimer.h"
#include <stdint.h>
#include <stdio.h>
#include <misc.h>

using namespace HAL;
namespace STM32F1
{
	F1Timer::F1Timer(TIM_TypeDef* TIMx)
	{	
		this->TIMx=TIMx;
	}
	void F1Timer::TimerInit(TIM_TypeDef* TIMx)
	{
		NVIC_InitTypeDef NVIC_InitStructure;
		if(TIM1==TIMx)
		{
			NVIC_InitStructure.NVIC_IRQChannel = IRQn = TIM1_UP_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
		}
		else if(TIM2==TIMx)
		{
			NVIC_InitStructure.NVIC_IRQChannel = IRQn = TIM2_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
		}
		else if(TIM3==TIMx)
		{
			NVIC_InitStructure.NVIC_IRQChannel = IRQn = TIM3_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
		}
		else if(TIM4==TIMx)
		{
			NVIC_InitStructure.NVIC_IRQChannel = IRQn = TIM4_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
		}
#ifdef STM32F10X_HD
		else if(TIM5==TIMx)
		{
			NVIC_InitStructure.NVIC_IRQChannel = IRQn = TIM5_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
		}
		else if(TIM6==TIMx)
		{
			/*
			NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;		// TODO: IRQ?
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
			*/
			printf("TIM6 not supported\n");
		}
		else if(TIM7==TIMx)
		{
			NVIC_InitStructure.NVIC_IRQChannel = IRQn = TIM7_IRQn;		// TODO: IRQ?
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
		}
#endif
	}
	void F1Timer::set_period(uint32_t period)
	{
		if(TIM1==TIMx)
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
		if(TIM2==TIMx)
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
		if(TIM3==TIMx)
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
		if(TIM4==TIMx)
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
		if(TIM5==TIMx)
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
		if(TIM6==TIMx)
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
		if(TIM7==TIMx)
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		TIM_DeInit(TIMx);
		TIM_InternalClockConfig(TIMx);
		TIM_TimeBaseStructure.TIM_Prescaler= 71;//1Mhz 1us 65536
		TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_Period=period-1;
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
		TIM_TimeBaseInit(TIMx,&TIM_TimeBaseStructure);
		TIM_ClearFlag(TIMx,TIM_FLAG_Update);
		TIM_ARRPreloadConfig(TIMx,DISABLE);
		TIM_ITConfig(TIMx,TIM_IT_Update,ENABLE);
		TIM_Cmd(TIMx,ENABLE);
		TimerInit(this->TIMx);
	}
	void F1Timer::set_callback(timer_callback cb, void *user_data)
	{		
		this->cb=cb;
		this->user_data = user_data;
	}
	
	void F1Timer::restart()
	{
		TIM_Cmd(TIMx,DISABLE);
		TIMx->CNT = 0;
		TIM_ClearITPendingBit(TIMx , TIM_FLAG_Update);
		__DSB();
		__ISB();
		__DMB();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		systimer->delayus(5);
		TIM_Cmd(TIMx,ENABLE);
	}
	void F1Timer::enable_cb()
	{
		NVIC_EnableIRQ(IRQn);
	}
	
	void F1Timer::disable_cb()
	{
		NVIC_DisableIRQ(IRQn);
	}
	
	void F1Timer::call_callback()
	{
		if(TIM1==TIMx)
			TIM_ClearITPendingBit(TIM1 , TIM_FLAG_Update);
		else if(TIM2==TIMx)
			TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		else if(TIM3==TIMx)
			TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
		else if(TIM4==TIMx)
			TIM_ClearITPendingBit(TIM4 , TIM_FLAG_Update);
		else if(TIM5==TIMx)
			TIM_ClearITPendingBit(TIM5 , TIM_FLAG_Update);
		else if(TIM6==TIMx)
			TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update);
		else if(TIM7==TIMx)
			TIM_ClearITPendingBit(TIM7 , TIM_FLAG_Update);
		else if(TIM8==TIMx)
			TIM_ClearITPendingBit(TIM8 , TIM_FLAG_Update);
		if(cb)
			cb(user_data);
	}
}
