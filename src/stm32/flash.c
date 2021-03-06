// CANnuccia/src/stm32/flash.c - STM32 implementation of common/flash.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/flash.h"

#include <stdint.h>

// See the STM32F10x Programming Manual, PM0075
// Assuming target is a medium-density (64kB) device

struct Flash
{
    uint32_t ACR;
    uint32_t KEYR;
    uint32_t OPTKEYR;
    uint32_t SR;
    uint32_t CR;
    uint32_t AR;
    uint32_t _; // (reserved)
    uint32_t OBR;
    uint32_t WRPR;
};
#define FLASH ((volatile struct Flash *)0x40022000)

#define FLASH_KEY1 0x45670123u
#define FLASH_KEY2 0xCDEF89ABu
#define FLASH_CR_LOCK 0x00000080u
#define FLASH_CR_STRT 0x00000040u
#define FLASH_CR_PER 0x00000002u
#define FLASH_CR_PG 0x00000001u
#define FLASH_SR_BSY 0x00000001u

#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08)

extern char _flash_start, _flash_end; // (defined in the linker script)


uintptr_t cnFlashSize(void)
{
    // NOTE: Could also query the flash size register
    return (uintptr_t)(&_flash_end - &_flash_start);
}

int cnFlashPageWriteable(uintptr_t addr)
{
    uintptr_t minAddr = (uintptr_t)(&_flash_start) + CN_FLASH_BOOTLOADER_SIZE;
    uintptr_t maxAddr = (uintptr_t)(&_flash_end);
    return addr >= minAddr && (addr + CN_FLASH_PAGE_SIZE) < maxAddr;
}

int cnFlashUnlock(void)
{
    // Write KEY1 then KEY2 to FLASH_KEYR to unlock FLASH_CR
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;

    if(FLASH->CR & FLASH_CR_LOCK)
    {
        // Unlocking failed. FLASH_CR will stay locked up to the next reset :(
        return 0;
    }
    return 1;
}

int cnFlashLock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
    return (FLASH->CR & FLASH_CR_LOCK);
}

/// The address of the page currently being programmed.
static uintptr_t curPageAddr = 0;

/// Spinlocks until flash is busy (`FLASH_SR_BSY`).
inline static void waitForFlash(void)
{
    while(FLASH->SR & FLASH_SR_BSY) { }
}

int cnFlashBeginWrite(uintptr_t addr)
{
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        // Flash locked, can't program it
        return 0;
    }

    // Clear page
    // TODO IMPLEMENT: bootloader protection (refuse to clear/write to pages
    //                 that belong to the bootloader, check `addr`)
    // FIXME IMPLEMENT: verify the page has been really cleared by reading it
    waitForFlash();
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = addr;
    FLASH->CR |= FLASH_CR_STRT;
    waitForFlash();
    FLASH->CR &= ~FLASH_CR_PER;

    // Start programming operation
    FLASH->CR |= FLASH_CR_PG;

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

    // NOTE: Must write exactly 16 bits (Half Word) at a time or a bus error occurs!
    volatile uint16_t *destHW = (volatile uint16_t *)(curPageAddr + offset);
    const uint16_t *srcHW = (const uint16_t *)data;
    unsigned bytesWritten;
    for(bytesWritten = 0; bytesWritten < size; bytesWritten += 2)
    {
        waitForFlash();
        *destHW++ = *srcHW++;
    }
    waitForFlash();

    // FIXME IMPLEMENT: verify the written data by reading it

    return bytesWritten;
}

int cnFlashEndWrite(void)
{
    if(!curPageAddr)
    {
        // `cnFlashBeginWrite()` has not been called
        return 0;
    }

    waitForFlash();
    FLASH->CR &= ~FLASH_CR_PG;

    curPageAddr = 0;
    return 1;
}

uint8_t cnReadDevId(void)
{
    return (FLASH->OBR & 0x0003FC00) >> 10; // data0: [10..17]
}


typedef void(*ResetHandler)(void);

__attribute__((noreturn)) void cnJumpToProgram(void)
{
    // FIXME IMPLEMENT: The code should undo all modifications done to system
    //                  registers before jumping to the user program. In particular:
    //                  - Disable GPIO ports (debug LED + bxCAN), AFIO, TIM2, other devices
    //                  - Disable all enabled interrupts (TIM2, bxCAN...)
    //                  - Reset the internal oscillator as clock source (no PLL)

    // The vector table of the user program is right after the bootloader's end.
    uintptr_t vtAddr = ((uintptr_t)&_flash_start) + CN_FLASH_BOOTLOADER_SIZE;
    volatile uint32_t *vt = (volatile uint32_t *)vtAddr;

    ResetHandler userReset = (ResetHandler)vt[1];

    // The stack pointer to use for the user program is the first entry of the
    // vector table.
    __asm__ __volatile__("MSR msp, %0"
                         : // (no outputs)
                         : "r"(vt[0])
                         :);

    // Switch to the user program's vector table.
    // -> http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHHDGBC.html
    SCB_VTOR = vtAddr;
    __asm__ __volatile__("DSB");

    // So long, and thanks for all the fish!
    userReset();

    __builtin_trap(); // (should never reach here)
}
