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

#ifndef CN_FLASH_PAGE_SIZE
#   error "CN_FLASH_PAGE_SIZE must be defined by the build system"
#endif

#ifndef CN_FLASH_BOOTLOADER_SIZE
#   error "CN_FLASH_BOOTLOADER_SIZE must be defined by the build system"
#endif


/// Unlocks flash memory for writing.
/// Returns true on success or false on error.
int cnFlashUnlock(void);

/// Locks flash memory, preventing any future writes to it.
/// Returns true on success or false on error.
int cnFlashLock(void);

/// Returns true if the page in flash that starts at `addr` (the address of
/// its first byte) is writeable, false if it should not be written to (it is
/// part of the bootloader, out-of-bounds, ...)
int cnFlashPageWriteable(uintptr_t addr);

/// Begins a write/erase cycle for the page starting at `addr` in flash, preparing
/// data to be filled in with `cnFlashFill()`.
/// Unlock flash with `cnFlashUnlock()` before use.
/// Returns true on success or false on error.
///
/// On STM32: unlocks flash for writing, erases the flash page at `addr` and
///           sets `FLASH_CR->PG`.
/// On AVR: erases the flash page at `addr`.
int cnFlashBeginWrite(uintptr_t addr);

/// Copies `size` bytes of `data`, offset by `offset` bytes into the page currently
/// being written to - see `cnFlashBeginWrite()`.
/// Returns the number of bytes actually copied (0 on error).
///
/// On STM32: writes `data` to `page address + offset` directly, in blocks of 16 bits.
/// On AVR: writes `data` to the internal scrap page, `offset` bytes into it, in blocks of 16 bits.
unsigned cnFlashFill(uintptr_t offset, unsigned size, const uint8_t data[size]);

/// Ends a write/erase cycle, committing the page write to flash.
/// Returns true on success or false on error.
///
/// On STM32: clears `FLASH_CR->PG`.
/// On AVR: copies the internal scrap page to the actual page to program in flash.
int cnFlashEndWrite(void);

/// Jumps from the bootloader to the user program.
void cnJumpToProgram(void);

#endif // FLASH_H
