// CANnuccia/src/avr/debug.c - AVR implementation of common/debug.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/debug.h"

#include <avr/io.h>

// Onboard LED is on PB5
#define LED_PORT_DIR DDRB
#define LED_PORT PORTB
#define LED_PIN_MASK 0x20

int cnDebugInit(void)
{
    LED_PORT_DIR |= LED_PIN_MASK;
    return 1;
}

void cnDebugLed(int on)
{
    if(on)
    {
        LED_PORT |= LED_PIN_MASK;
    }
    else
    {
        LED_PORT &= ~LED_PIN_MASK;
    }
}
