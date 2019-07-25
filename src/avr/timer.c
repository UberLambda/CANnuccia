// CANnuccia/src/avr/timer.c - AVR implementation of common/timer.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/timer.h"

#ifndef F_CPU
#   define F_CPU 16000000UL
#endif

#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/// The function to run on timer timeout, as set by `cnTimerStart()`.
static CNtimeoutFunc timeoutFunc = NULL;

/// Does the timer have to be stopped when the ISR is run?
static int timerOneshot = 0;

ISR(TIMER1_OVF_vect)
{
    if(timeoutFunc)
    {
        timeoutFunc();
    }

    if(timerOneshot)
    {
        cnTimerStop();
    }
}


int cnTimerStart(uint32_t delayUs, int oneshot, CNtimeoutFunc onTimeout)
{
    if(delayUs == 0 || !onTimeout)
    {
        // Invalid args
        return 0;
    }

    timeoutFunc = onTimeout;
    timerOneshot = oneshot;

    // Calculate timer 1's prescaler and count from delay;
    //
    //      timerFreq = F_CPU / (PSC * CNT)
    //   -> 1000000 / delayUs = F_CPU / (PSC * CNT)
    //   -> PSC * CNT = F_CPU / 1000000 * delayUs
    //   -> CNT = F_CPU / 1000000 * delayUs / PSC
    //
    // Hence try all possible prescalers (1, 8, 64, 256, 1024), stopping at the
    // first that works. If none works the delay is too big...
    static const uint32_t PSCS[] = { 1, 8, 64, 256, 1024 };
    static const uint8_t PSC_REGS[] = { // (values to set TCCR1B to for a prescaler)
        (1 << CS10), // 1
        (1 << CS11), // 8
        (1 << CS10) | (1 << CS11), // 64
        (1 << CS12), // 256
        (1 << CS12) | (1 << CS10), // 1024
    };
    static const unsigned N_PSCS = sizeof(PSCS) / sizeof(PSCS[0]);

    uint32_t cnt = 0xFFFFUL;
    int pscChosen = 0;
    unsigned pscId;
    for(pscId = 0; pscId < N_PSCS; pscId ++)
    {
        cnt = F_CPU / 1000000UL * delayUs / PSCS[pscId];
        if(cnt <= 0xFFFFUL)
        {
            pscChosen = 1;
            break;
        }
    }
    if(!pscChosen)
    {
        // Too big of a delay, settle for the maximum prescaler and count possible
        cnt = 0xFFFFUL;
        pscId = (N_PSCS - 1);
    }

    // Enable timer interrupt, set target count, set prescaler (triggering the timer)
    TIMSK1 |= (1 << TOIE1);
    TCNT1 = (cnt & 0xFFFFUL);
    TCCR1B = PSC_REGS[pscId];
    sei();

    return 1;
}

void cnTimerStop(void)
{
    // Disable timer 1 and its interrupts
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TIMSK1 &= ~(1 << TOIE1);
    TCNT1 = 0x0000;

    timeoutFunc = NULL;
}
