// CANnuccia/src/avr/flash.c - AVR implementation of common/flash.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/flash.h"

#include <avr/boot.h>
#include <avr/interrupt.h>

// FIXME: Values are hardcoded for ATMega328p!
#define FLASH_SIZE 0x8000 // 32kB

uintptr_t cnFlashSize(void)
{
    return FLASH_SIZE;
}

int cnFlashPageWriteable(uintptr_t addr)
{
    // On AVR the application goes from 0x0000 to the start of the bootloader;
    // the bootloader is at the end of flash.
    return (addr + CN_FLASH_PAGE_SIZE) < (FLASH_SIZE - CN_FLASH_BOOTLOADER_SIZE);
}

/// A software "lock" for flash memory.
static int flashLocked = 1;

int cnFlashUnlock(void)
{
    // TODO: Set lock bits to protect the bootloader here

    flashLocked = 0;
    return 1;
}

int cnFlashLock(void)
{
    flashLocked = 1;
    return 1;
}

/// The address of the page currently being programmed.
static uintptr_t curPageAddr = 0;

int cnFlashBeginWrite(uintptr_t addr)
{
    curPageAddr = addr;
    return 1;
}

unsigned cnFlashFill(uintptr_t offset, unsigned size, const uint8_t data[size])
{
    if(!curPageAddr)
    {
        // `cnFlashBeginWrite()` has not been called
        return 0;
    }

    // Fill the bootloader temporary page buffer with the data, one 16-bit word
    // at a time; MSB is written to the highest address, LSB to the lowest.
    const uint16_t *src = (const uint16_t *)data;

    unsigned bytesWritten, addr = curPageAddr + offset; // Byte address where to write in flash
    for(bytesWritten = 0;
        bytesWritten < size && (bytesWritten + offset) < CN_FLASH_PAGE_SIZE;
        bytesWritten += 2, addr += 2)
    {
        uint16_t word = *src++; // NOTE: word will be little endian
        boot_page_fill_safe(addr, word);
    }

    return bytesWritten;
}

int cnFlashEndWrite(void)
{
    if(!curPageAddr || flashLocked)
    {
        // `cnFlashBeginWrite()` has not been called or flash is locked
        return 0;
    }

    // Disable interrupts for safety
    uint8_t sregBak = SREG;
    cli();

    // Copy the bootloader temporary page buffer to the target page
    boot_page_erase_safe(curPageAddr);
    boot_page_write_safe(curPageAddr);

    // Re-enable interrupts
    SREG = sregBak;

    curPageAddr = 0;
    return 1;
}


typedef void(*ResetHandler)(void);

__attribute__((noreturn)) void cnJumpToProgram(void)
{
    // FIXME IMPLEMENT: Reset all modified registers to their default values:
    // - Disable the debug LED GPIO pin
    // - Disable SPI (used for the CAN chip)
    // - Reset the timer's registers
    //
    // Could use the "watchdog timer reset" trick to do so.

    // Re-enable the RWW section as we have to boot from it
    boot_rww_enable_safe();

    // See you on the other side...
    __asm__ __volatile__("JMP 0");

    __builtin_trap(); // (should never reach here)
}
