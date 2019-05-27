// CANnuccia/src/common/util.h - Inline functions and interfaces for utilities
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

/// Reads a little-endian U32 from 4 bytes.
inline uint32_t readU32LE(const uint8_t bytes[4])
{
    return bytes[0]
            | ((uint32_t)bytes[1] << 8)
            | ((uint32_t)bytes[2] << 16)
            | ((uint32_t)bytes[3] << 24);
}

/// Converts a little-endian U32 to 4 bytes.
inline void writeU32LE(uint8_t outBytes[4], uint32_t u32)
{
    outBytes[0] = (u32 & 0x000000FFu);
    outBytes[1] = (u32 & 0x0000FF00u) >> 8;
    outBytes[2] = (u32 & 0x00FF0000u) >> 16;
    outBytes[3] = (u32 & 0xFF000000u) >> 24;
}

#endif // UTIL_H
