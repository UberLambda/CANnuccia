// CANnuccia/src/avr/flash.c - AVR implementation of common/flash.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/flash.h"

#include <stdint.h>

uintptr_t cnFlashSize(void)
{
    // FIXME IMPLEMENT!
    return 0;
}

int cnFlashPageWriteable(uintptr_t addr)
{
    // FIXME IMPLEMENT!
    return 0;
}

int cnFlashUnlock(void)
{
    // FIXME IMPLEMENT!
    return 0;
}

int cnFlashLock(void)
{
    // FIXME IMPLEMENT!
    return 0;
}

/// The address of the page currently being programmed.
static uintptr_t curPageAddr = 0;

int cnFlashBeginWrite(uintptr_t addr)
{
    // FIXME IMPLEMENT!
    return 0;
}

unsigned cnFlashFill(uintptr_t offset, unsigned size, const uint8_t data[size])
{
    if(!curPageAddr)
    {
        // `cnFlashBeginWrite()` has not been called
        return 0;
    }

    // FIXME IMPLEMENT!
    return 0;
}

int cnFlashEndWrite(void)
{
    if(!curPageAddr)
    {
        // `cnFlashBeginWrite()` has not been called
        return 0;
    }

    // FIXME IMPLEMENT!
    return 0;
}


__attribute__((noreturn)) void cnJumpToProgram(void)
{
    // FIXME IMPLEMENT!

    __builtin_trap(); // (should never reach here)
}
