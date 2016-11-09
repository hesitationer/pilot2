#pragma once

#include <HAL/Interface/ISPI.h>
#include <HAL/Interface/IGPIO.h>
#include <HAL/Interface/IInterrupt.h>
#include <HAL/Interface/ITimer.h>

extern HAL::ISPI *spi;
extern HAL::IGPIO *cs;
extern HAL::IGPIO *ce;
extern HAL::IGPIO *irq;
extern HAL::IGPIO *dbg;
extern HAL::IGPIO *dbg2;
extern HAL::IInterrupt *interrupt;
extern HAL::ITimer *timer;

extern HAL::IGPIO *SCL;
extern HAL::IGPIO *SDA;

int board_init();
