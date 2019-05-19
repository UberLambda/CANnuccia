/*
 * STM32F103C8 linker script.
 *
 *******************************************************************************
 * Based on: https://github.com/al95/STM32-Bare-Metal/blob/master/STM32F103C8.ld
 *                                                                             *
 * Copyright (c) 2017 Andrea Loi                                               *
 *                                                                             *
 * Permission is hereby granted, free of charge, to any person obtaining a     *
 * copy of this software and associated documentation files (the "Software"),  *
 * to deal in the Software without restriction, including without limitation   *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
 * and/or sell copies of the Software, and to permit persons to whom the       *
 * Software is furnished to do so, subject to the following conditions:        *
 *                                                                             *
 * The above copyright notice and this permission notice shall be included     *
 * in all copies or substantial portions of the Software.                      *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL     *
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
 * DEALINGS IN THE SOFTWARE.                                                   *
 *                                                                             *
 *******************************************************************************
 */

/* The ISR run when the chip is reset. */
ENTRY(resetHandler);

MEMORY
{
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
    /* ARM ISR vector table.
     * See: https://developer.arm.com/docs/dui0552/latest/the-cortex-m3-processor/exception-model/vector-table
     *
     * BOOT0 should be low; this way, VTOR will be set appropriately so that the
     * interrupt vector table is expected to be at the start of the flash memory.
     */
    .isrs :
    {
        . = ORIGIN(FLASH);
        KEEP(*(.isrs));
    } >FLASH

    /* Program code + const data. Loaded directly from flash. */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    /* For C++, .ARM.extab and .ARM.exidx would go here (required for stack unwinding) */

    /* Initialized R/W data.
     * Loaded from flash, needs to be copied to RAM on chip reset.
     */
    .data :
    {
        . = ALIGN(4);
        _data_start = .;
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } >RAM AT >FLASH

    /* On chip reset, `_data_start` to `_data_end` must be copied to `_data_load_addr`
     * (see .data section above)
     */
    _data_load_addr = LOADADDR(.data);

    /* Uninitialized or zero-filled R/W data. Basically just a zero-filled chunk
     * of RAM; holds uninitialized variables or variables initialized to only
     * contains zeroes (ex. `long foo = 0;`).
     * Needs to be zero-filled by the program on chip reset.
     */
    .bss :
    {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } >RAM
}

