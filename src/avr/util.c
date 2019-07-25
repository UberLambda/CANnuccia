// CANnuccia/src/avr/util.c - AVR implementation of common/util.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/util.h"

#include <util/crc16.h>

uint16_t cnCRC16(unsigned len, const uint8_t data[len])
{
    uint16_t crc = 0x0000; // (as per XModem CRC16)
    for(unsigned i = 0; i < len; i ++)
    {
        crc = _crc_xmodem_update(crc, data[i]);
    }
    return crc;
}
