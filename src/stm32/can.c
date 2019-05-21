// CANnuccia/src/stm32/can.c - STM32 implementation of common/can.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/can.h"

int cnCANInit(uint32_t id, uint32_t mask)
{
    // FIXME IMPLEMENT!
    return 0;
}

int cnCANSend(uint32_t id, unsigned len, const uint8_t data[len])
{
    // FIXME IMPLEMENT!
    return -1;
}

int cnCANRecv(uint32_t *recvId, unsigned maxLen, uint8_t data[maxLen])
{
    // FIXME IMPLEMENT!
    return -1;
}
