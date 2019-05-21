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


const unsigned CN_FLASH_PAGE_SIZE = 1024; // 1kB pages on STM32 medium-density


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

void cnJumpToProgram(void)
{
    // FIXME: Implement!
}
