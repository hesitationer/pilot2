#include "board.h"

#include <HAL/STM32F1/F1SPI.h>
#include <HAL/STM32F1/F1GPIO.h>
#include <HAL/STM32F1/F1Interrupt.h>
#include <misc.h>
#include <HAL/STM32F1/F1Timer.h>

using namespace STM32F1;
using namespace HAL;


F1GPIO _cs(GPIOB, GPIO_Pin_5);
F1GPIO _ce(GPIOB, GPIO_Pin_4);
F1GPIO _irq(GPIOB, GPIO_Pin_1);
F1GPIO _dbg(GPIOB, GPIO_Pin_6);

F1GPIO _dbg2(GPIOC, GPIO_Pin_5);
F1GPIO _SCL(GPIOC, GPIO_Pin_13);
F1GPIO _SDA(GPIOC, GPIO_Pin_14);

F1SPI _spi;
F1Interrupt _interrupt;
F1Timer _timer(TIM2);

int board_init()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	
	cs = &_cs;
	ce = &_ce;
	irq = &_irq;
	dbg = &_dbg;
	dbg2 = &_dbg2;
	SCL = &_SCL;
	SDA = &_SDA;
	spi = &_spi;
	interrupt = &_interrupt;
	timer = &_timer;
	
	_spi.init(SPI1);
	_interrupt.init(GPIOB, GPIO_Pin_1, interrupt_falling);
	
	return 0;
}

extern "C" void TIM2_IRQHandler(void)
{
	_timer.call_callback();
}

void read_channels(short *data, int channel_count)
{
}
