#pragma once

#include <HAL/Interface/ISPI.h>
#include <HAL/Interface/IGPIO.h>
#include <HAL/Interface/IInterrupt.h>
#include <HAL/Interface/ITimer.h>
#include <HAL/Interface/IRCOUT.h>
#include <HAL/Interface/IUART.h>
#include <string.h>
#include <Protocol/crc32.h>

extern HAL::ISPI *spi;
extern HAL::IGPIO *cs;
extern HAL::IGPIO *ce;
extern HAL::IGPIO *irq;
extern HAL::IGPIO *dbg;
extern HAL::IGPIO *dbg2;
extern HAL::IInterrupt *interrupt;
extern HAL::ITimer *timer;
extern HAL::IGPIO *bind_button;

extern HAL::IGPIO *SCL;
extern HAL::IGPIO *SDA;
extern HAL::IRCOUT *ppm;
extern HAL::IUART *uart;
extern HAL::IGPIO *vibrator;

typedef struct 
{
	unsigned _min:12;
	unsigned _max:12;
	unsigned middle:12;
	unsigned dead_band:7;
	unsigned reverse:1;
} configure_entry;

int board_init();
void read_channels(int16_t *channel, int max_channel_count);
void select_ant(uint32_t *randomizer, bool tx);		// 16bytes randomizer

static uint64_t board_get_seed()
{
	const void *stm32_id_address = (const void*)0x1FFFF7E8;
	char data[12];
	memcpy(data, stm32_id_address, 12);

	uint64_t o;	
	((uint32_t*)&o)[0] = crc32(0, data, 8);
	((uint32_t*)&o)[1] = crc32(0, data+4, 8);
	return o;
}

