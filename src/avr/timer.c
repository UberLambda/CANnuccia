// CANnuccia/src/avr/timer.c - AVR implementation of common/timer.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/timer.h"

#include <stddef.h>

/// The function to run on timer timeout, as set by `cnTimerStart()`.
static CNtimeoutFunc timeoutFunc = NULL;

int cnTimerStart(uint32_t delayUs, int oneshot, CNtimeoutFunc onTimeout)
{
    if(delayUs == 0 || !onTimeout)
    {
        // Invalid args
        return 0;
    }

    // FIXME IMPLEMENT!
    return 0;
}

void cnTimerStop(void)
{
    // FIXME IMPLEMENT!
}
