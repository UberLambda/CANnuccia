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
    uint8_t data[] = {1, 2, 3, 4, 6, 8, 10, 12}, data2[] = {0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF};

    volatile int i = 0;
    uint32_t recvId = 0;
    while(1)
    {
        cnCANSend(0xF0F0F0F4, sizeof(data), data);
        if(cnCANRecv(&recvId, sizeof(data2), data2) >= 0)
        {
            cnDebugLed(i % 2);
            i ++;
        }
        for(volatile int j = 0; j < 200000; j ++) { }
    }
}
