// CANnuccia/src/avr/debug.c - AVR implementation of common/debug.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/debug.h"

#include "common/cc.h"

// No debug LED is present; the LED on PORTB5 is on the same pin as SPI's SCK,
// so it can't be used as GPIO.

int cnDebugInit(void)
{
    // Onboard LED not present.
    return 1;
}

void cnDebugLed(int on)
{
    // Onboard LED not present.
    CN_UNUSED(on);
}
