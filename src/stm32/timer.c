// CANnuccia/src/stm32/timer.c - Implementation of common/timer.h for STM32
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/timer.h"

#include <stddef.h>

// TIM2..5 are present on all STM32F10x and are general-purpose 16-bit timers
// See STM32's reference manual, general-purpose timers section
struct Tim
{
    uint32_t CR1;
    uint32_t CR2;
    uint32_t SMCR;
    uint32_t DIER;
    uint32_t SR;
    uint32_t EGR;
    uint32_t CCMR1;
    uint32_t CCMR2;
    uint32_t CCER;
    uint32_t CNT;
    uint32_t PSC;
    uint32_t ARR;
    uint32_t _reserved1[1];
    uint32_t CCR1;
    uint32_t CCR2;
    uint32_t CCR3;
    uint32_t CCR4;
    uint32_t _reserved2[1];
    uint32_t DCR;
    uint32_t DMAR;
};
#define TIM2 ((volatile struct Tim *)0x40000000)
#define TIM_SR_UIF 0x00000001u
#define TIM_CR1_ARPE 0x00000080u
#define TIM_CR1_DIR 0x00000010u
#define TIM_CR1_OPM 0x00000008u
#define TIM_CR1_URS 0x00000004u
#define TIM_CR1_CEN 0x00000001u
#define TIM_DIER_UIE 0x00000001u
#define TIM_EGR_UG 0x00000001u

#define RCC_APB1ENR (*(volatile uint32_t *)0x4002101C)
#define RCC_APB1ENR_TIM2ENR 0x00000001u

// TIM2 interrupt is #28 -> set the 28th bit of ISER0
#define TIM2_IRQN 28
#define NVIC_ISER0 (*(volatile uint32_t *)0xE000E100)
#define NVIC_ICER0 (*(volatile uint32_t *)0xE000E180)
#define NVIC_ICPR0 (*(volatile uint32_t *)0xE000E280)

#define CLOCK_FREQ_MHZ 72


/// The function to run on timer timeout, as set by `cnTimerStart()`.
static CNtimeoutFunc timeoutFunc = NULL;

/// The ISR registered in startup.c's vector table.
void tim2Handler(void)
{
    TIM2->SR &= ~TIM_SR_UIF; // Clear UIF or the code will get stuck in this ISR!
    timeoutFunc();
}

int cnTimerStart(uint32_t delayUs, int oneshot, CNtimeoutFunc onTimeout)
{
    if(delayUs == 0 || !onTimeout)
    {
        // Invalid args
        return 0;
    }

    RCC_APB1ENR |= RCC_APB1ENR_TIM2ENR; // Enable TIM2's clock
    TIM2->CR1 &= ~TIM_CR1_CEN; // Ensure TIM2's counter is stopped
    TIM2->CR1 |= TIM_CR1_DIR; // TIM2 counts down
    TIM2->CR1 |= TIM_CR1_URS; // TIM2 updates trigger only on overflow or underflow

    // Set TIM2's counter reload value and prescaler;
    //
    //      timerFreq = 72MHz / ((PSC + 1) * (ARR + 1))
    //   -> 1000000 / delayUs = 72MHz / ((PSC + 1) * (ARR + 1))
    //   -> (PSC + 1) * (ARR + 1) = 72MHz * delayUs / 1000000
    //   -> (PSC + 1) * (ARR + 1) = 72 * delayUs
    //   -> ARR = 72 * delayUs / (PSC + 1) - 1
    // also ARR <= 65535, so
    //      72 * delayUs / (PSC + 1) - 1 <= 65535
    //   -> 72 * delayUs <= 65536 * (PSC + 1)
    //
    // Hence try all prescalers from 1 to 65535, stopping at the first that works.
    // If none works the delay is too big, so we settle for PSC=65535 and ARR=65535.
    // It is not likely to be the most accurate prescaler but it should be good
    // enough.
    int psc, arr = 0xFFFF, delayUsI = (int)delayUs;
    for(psc = 0; psc <= 0xFFFF; psc ++)
    {
        if((CLOCK_FREQ_MHZ * delayUsI) <= 0x10000 * (psc + 1))
        {
            arr = CLOCK_FREQ_MHZ * delayUsI / (psc + 1) - 1;
            if(arr > 0 && arr <= 0xFFFF)
            {
                break;
            }
        }
    }
    TIM2->PSC = psc & 0xFFFF;
    TIM2->ARR = arr & 0xFFFF;
    TIM2->EGR |= TIM_EGR_UG; // Update Generation, applies the new ARR.
                             // IMPORTANT or the interrupt will spuriously trigger
                             // as soon as the timer is enabled!

    timeoutFunc = onTimeout; // Set the function the `tim2Handler()` ISR will call
    TIM2->CR1 |= oneshot ? TIM_CR1_OPM : 0; // Set oneshot or repeating
    TIM2->DIER |= TIM_DIER_UIE; // Make TIM2 trigger update interrupts
    NVIC_ICPR0 |= (1 << TIM2_IRQN); // Clear any pending TIM2 interrupt
    NVIC_ISER0 |= (1 << TIM2_IRQN); // Enable the TIM2 interrupt vector

    TIM2->CR1 |= TIM_CR1_CEN; // Start TIM2's counter
    return 1;
}

void cnTimerStop(void)
{
    TIM2->CR1 &= ~TIM_CR1_CEN; // Stop TIM2's counter
    TIM2->DIER &= ~TIM_DIER_UIE; // Stop TIM2 from triggering update interrupts
    NVIC_ICER0 |= (1 << TIM2_IRQN); // Disable the TIM2 interrupt vector
    RCC_APB1ENR &= ~RCC_APB1ENR_TIM2ENR; // Disable TIM2's clock
}
