// CANnuccia/src/stm32/debug.c - STM32 implementation of common/debug.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/debug.h"

#include <stdint.h>

// See the STM32F10X manual: RCC section and GPIO section
// STM32 blue pill: debug LED on PC13, active low

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018)
#define RCC_APB2ENR_IOPCEN 0x00000010u
#define GPIOC_CRH (*(volatile uint32_t *)0x40011004)
#define GPIOC_BSRR (*(volatile uint32_t *)0x40011010)

int cnDebugInit(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPCEN; // Enable clock source for GPIO port C
    GPIOC_CRH |= (0x1 << 20); // Set PC13 as output. CNF13=00=(GPIO push/pull), MODE13=10=(max speed 2MHz)
    return 1;
}

void cnDebugLed(int on)
{
    GPIOC_BSRR = on ? 0x20000000u : 0x00002000u; // Turn PC13 on or off (active low)
}
