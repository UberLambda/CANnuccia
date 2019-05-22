// CANnuccia/src/common/main.c - Entry point of CANnuccia
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "common/debug.h"
#include "common/can.h"

int main(void)
{
    cnDebugInit();

    cnCANInit(0x00000000, 0x00000000);

    volatile int i = 0;
    while(1)
    {
        for(volatile int j = 0; j < 200000; j ++) { }
        cnDebugLed(i % 2);
        i ++;
    }
}
