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
#define FLASH_CR_PG 0x00000001u
#define FLASH_SR_BSY 0x00000001u

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

int cnFlashWrite(uintptr_t addr, unsigned count, const uint16_t data[count])
{
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        // Flash locked, can't program it
        return 0;
    }
    FLASH->CR |= FLASH_CR_PG;

    // NOTE: Must write exactly 16 bits (half word) at a time or a bus error occurs
    volatile uint16_t *dest = (volatile uint16_t *)addr;
    const uint16_t *src = data;
    unsigned written;
    for(written = 0; written < count; written ++)
    {
        while(FLASH->SR & FLASH_SR_BSY) { }
        *dest++ = *src++;
    }
    while(FLASH->SR & FLASH_SR_BSY) { }

    FLASH->CR ^= FLASH_CR_PG; // (clear PG bit)
    return 1;
}

void cnJumpToProgram(void)
{
    // FIXME: Implement!
}
