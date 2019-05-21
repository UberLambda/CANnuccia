// CANnuccia/src/common/flash.h - The interface to flash I/O on target MCUs
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

/// Unlocks flash memory for writing.
/// Returns true on success or false on error.
int cnFlashUnlock(void);

/// Locks flash memory, preventing any future writes to it.
/// Returns true on success or false on error.
int cnFlashLock(void);

/// Writes `count` 16-bit words of `data` to flash, at `addr`.
/// Unlock flash with `cnFlashUnlock()` before use.
/// Returns true on success or false on error.
int cnFlashWrite(uintptr_t addr, unsigned count, const uint16_t data[count]);

/// Jumps from the bootloader to the user program.
void cnJumpToProgram(void);

#endif // FLASH_H
