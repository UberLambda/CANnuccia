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

#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xCDEF89AB
#define FLASH_CR_LOCK (1 << 7)

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

int cnFlashWrite(uintptr_t addr, unsigned len, const uint8_t data[len])
{
    // FIXME: Implement!
    return -1;
}

void cnJumpToProgram(void)
{
    // FIXME: Implement!
}
