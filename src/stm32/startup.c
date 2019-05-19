// CANnuccia/src/stm32/startup.c - STM32 vector table and core startup ISRs
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// *****************************************************************************
// Based on code from https://github.com/al95/STM32-Bare-Metal/blob/master/init.c
//                                                                             
// Copyright (c) 2017 Andrea Loi                                               
//                                                                             
// Permission is hereby granted, free of charge, to any person obtaining a     
// copy of this software and associated documentation files (the "Software"),  
// to deal in the Software without restriction, including without limitation   
// the rights to use, copy, modify, merge, publish, distribute, sublicense,    
// and/or sell copies of the Software, and to permit persons to whom the       
// Software is furnished to do so, subject to the following conditions:        
//                                                                             
// The above copyright notice and this permission notice shall be included     
// in all copies or substantial portions of the Software.                      
//                                                                             
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL     
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         
// DEALINGS IN THE SOFTWARE.                                                   
// *****************************************************************************

#include "common/cc.h"

/// An ISR that spinlocks forever.
/// Used as a fallback for when no real ISR is implemented.
CN_NORETURN void hcf(void)
{
    while(1)
    {
    }
}

int main(void);

/// Executed every time the chip is reset.
/// NOTE: `ENTRY(resetHandler)` in the linker script
CN_NORETURN void resetHandler(void)
{
    // v- Defined in linker script -v
    extern char _data_start; // ORIGIN(.data)
    extern char _data_end; // ORIGIN(.data) + LENGTH(.data)
    extern char _data_load_addr; // LOADADDR(.data)
    extern char _bss_start; // ORIGIN(.bss)
    extern char _bss_end; // ORIGIN(.bss) + LENGTH(.bss)

    // Enable 8-byte stack alignment to comply with AAPCS
    //BIT_SET(SCB->CCR, BIT_9);

    // Copy .data from FLASH to RAM
    char *src = &_data_load_addr, *dst = &_data_start, *end = &_data_end;
    while(dst < end)
    {
        *dst++ = *src++;
    }

    // Zero-fill .bss on RAM
    dst = &_bss_start; end = &_bss_end;
    while(dst < end)
    {
        *dst++ = 0;
    }

    main();
    hcf();
}

/// The stack's start address. Stack starts at the bottom of RAM and grows up.
/// 0x20005000: RAM bottom (0x20000000) + RAM size (0x5000, i.e. 20KB)
#define STACK_START_ADDR 0x20005000

/// ARM Cortex-M3 Interrupt vector table.
typedef void(*ISR)(void);
ISR isrs[] CN_SECTION(".isrs") =
{
    (ISR)STACK_START_ADDR, // Initial stack pointer

    // === ARM vectors =========================================================
    resetHandler,          // Reset
    hcf,                   // Non-maskable interrupt (NMI)
    hcf,                   // Hard fault
    hcf,                   // Memory management fault
    hcf,                   // Bus fault
    hcf,                   // Usage fault
    hcf,                   // (reserved)
    hcf,                   // (reserved)
    hcf,                   // (reserved)
    hcf,                   // (reserved)
    hcf,                   // Supervisor Call (SVCall)
    hcf,                   // Debug monitor
    hcf,                   // (reserved)
    hcf,                   // PendSV (used for context switching) 
    hcf,                   // SysTick (system timer reached zero)

    // === STM32-specific vectors ==============================================
    hcf,                  // WWDG
    hcf,                  // PVD
    hcf,                  // TAMPER
    hcf,                  // RTC
    hcf,                  // FLASH
    hcf,                  // RCC
    hcf,                  // EXTI0         
    hcf,                  // EXTI1         
    hcf,                  // EXTI2         
    hcf,                  // EXTI3         
    hcf,                  // EXTI4         
    hcf,                  // DMA1_Channel1 
    hcf,                  // DMA1_Channel2 
    hcf,                  // DMA1_Channel3 
    hcf,                  // DMA1_Channel4 
    hcf,                  // DMA1_Channel5 
    hcf,                  // DMA1_Channel6 
    hcf,                  // DMA1_Channel7 
    hcf,                  // ADC1_2        
    hcf,                  // USB_HP_CAN_TX 
    hcf,                  // USB_LP_CAN_RX0
    hcf,                  // CAN_RX1       
    hcf,                  // CAN_SCE       
    hcf,                  // EXTI9_5       
    hcf,                  // TIM1_BRK      
    hcf,                  // TIM1_UP       
    hcf,                  // TIM1_TRG_COM  
    hcf,                  // TIM1_CC       
    hcf,                  // TIM2          
    hcf,                  // TIM3          
    hcf,                  // TIM4          
    hcf,                  // I2C1_EV       
    hcf,                  // I2C1_ER       
    hcf,                  // I2C2_EV       
    hcf,                  // I2C2_ER       
    hcf,                  // SPI1          
    hcf,                  // SPI2          
    hcf,                  // USART1        
    hcf,                  // USART2        
    hcf,                  // USART3        
    hcf,                  // EXTI15_10     
    hcf,                  // RTCAlarm      
    hcf,                  // USBWakeup     
    hcf,                  // TIM8_BRK      
    hcf,                  // TIM8_UP       
    hcf,                  // TIM8_TRG_COM  
    hcf,                  // TIM8_CC       
    hcf,                  // ADC3          
    hcf,                  // FSMC          
    hcf,                  // SDIO          
    hcf,                  // TIM5          
    hcf,                  // SPI3          
    hcf,                  // UART4         
    hcf,                  // UART5         
    hcf,                  // TIM6          
    hcf,                  // TIM7          
    hcf,                  // DMA2_Channel1 
    hcf,                  // DMA2_Channel2 
    hcf,                  // DMA2_Channel3 
    hcf,                  // DMA2_Channel4 
    hcf,                  // DMA2_Channel5 
    hcf,                  // ETH           
    hcf,                  // ETH_WKUP      
    hcf,                  // CAN2_TX       
    hcf,                  // CAN2_RX0      
    hcf,                  // CAN2_RX1      
    hcf,                  // CAN2_SCE      
    hcf,                  // OTG_FS        
};



