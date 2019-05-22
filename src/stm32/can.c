// CANnuccia/src/stm32/can.c - STM32 implementation of common/can.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/can.h"

// See the STM32F10X manual: RCC, AFIO & pin remapping, and bxCAN
// CAN == CAN1 (CAN2 is present only on connectivity line MCUs)

#define RCC_APB1ENR (*(volatile uint32_t *)0x4002101C)
#define RCC_APB1ENR_CANEN 0x02000000u
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018)
#define RCC_APB2ENR_IOPBEN 0x00000008u
#define RCC_APB2ENR_AFIOEN 0x00000001u
#define AFIO_MAPR (*(volatile uint32_t *)0x40010004)
#define AFIO_MAPR_CAN1_PA11A12 0x00000000u // Map CAN1_RX to PA11, CAN1_TX to PA12
#define AFIO_MAPR_CAN1_PB8B9 0x00004000u // Map CAN1_RX to PB8, CAN1_TX to PB9
#define GPIOB_CRH (*(volatile uint32_t *)0x40010C04)

struct CanMailbox
{
    uint32_t IR;
    uint32_t DTR;
    uint32_t DLR;
    uint32_t DHR;
};

struct CanFilter
{
    uint32_t R1;
    uint32_t R2;
};

struct Can
{
    uint32_t MCR;
    uint32_t MSR;
    uint32_t TSR;
    uint32_t RF0R;
    uint32_t RF1R;
    uint32_t IER;
    uint32_t ESR;
    uint32_t BTR;
    uint32_t _reserved1[88];
    struct CanMailbox OUTBOX[3]; // 3 TX mailboxes
    struct CanMailbox INBOX[2]; // 2 RX mailboxes: FIFO0, FIFO1
    uint32_t _reserved2[12];
    uint32_t FMR;
    uint32_t FM1R;
    uint32_t _reserved3[1];
    uint32_t FS1R;
    uint32_t _reserved4[1];
    uint32_t FFA1R;
    uint32_t _reserved5[1];
    uint32_t FA1R;
    uint32_t _reserved6[8];
    struct CanFilter FILTER[27];
};
#define CAN1 ((volatile struct Can *)0x40006400)
#define CAN_TIR_TXRQ 0x00000001u
#define CAN_MCR_ABOM 0x00000040u
#define CAN_MCR_AWUM 0x00000020u
#define CAN_MCR_INRQ 0x00000001u
#define CAN_MSR_SLAK 0x00000002u
#define CAN_MSR_INAK 0x00000001u
#define CAN_TSR_TME2 0x10000000u
#define CAN_TSR_TME1 0x08000000u
#define CAN_TSR_TME0 0x04000000u
#define CAN_TSR_CODE 0x03000000u
#define CAN_FMR_FINIT 0x00000001u


/// Sets the CAN1 filter number `n` to the given 32-bit id & mask pair.
inline void initCANFilter(unsigned n, uint32_t id, uint32_t mask)
{
    const uint32_t fltBit = (1u << n);
    CAN1->FM1R &= ~fltBit; // CAN1: filter n in mask mode
    CAN1->FS1R |= fltBit; // CAN1: filter n is 32-bit (not 16-bit)
    CAN1->FFA1R &= ~fltBit; // CAN1: filter n assigned to FIFO 0
    CAN1->FILTER[n].R1 = id;
    CAN1->FILTER[n].R2 = mask;
    CAN1->FA1R = fltBit; // CAN1: filter 0 is active
}

int cnCANInit(uint32_t id, uint32_t mask)
{
    // TODO: Macro to set whether to remap CAN1 to port B or leave it on port A
    //       The code below makes CAN1 be mapped to port B
    RCC_APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN; // Enable clock source for GPIO port B and AFIO
    AFIO_MAPR |= AFIO_MAPR_CAN1_PB8B9; // Set CAN1 remapping to PB8/PB9
    GPIOB_CRH |= 0x94; // Set PB8 as floating input (CNF8=01=(floating input), MODE8=00=(input))
                       // Set PB9 as push-pull output (CNF9=10=(AF push/pull), MODE9=01=(output, max 10MHz))
    RCC_APB1ENR |= RCC_APB1ENR_CANEN; // Enable clock source for CAN1

    // TODO IMPLEMENT: enable CAN1 interrupts on msg received + sent?

    CAN1->MCR |= CAN_MCR_INRQ; // Ask CAN1 to enter init mode
    while(!(CAN1->MSR & CAN_MSR_INAK)) { } // Wait for CAN1 to actually enter init mode

    CAN1->FMR |= CAN_FMR_FINIT; // Enter filter init mode
    initCANFilter(0, id, mask);
    CAN1->FMR &= ~CAN_FMR_FINIT; // Exit filter init mode

    CAN1->MCR |= CAN_MCR_AWUM | CAN_MCR_ABOM; // Auto wakeup on message rx, auto bus-off on 128 errors
    // TODO: set other CAN options if needed (NART, RFLM, TXFP, ABOM...)

    CAN1->MCR &= ~CAN_MCR_INRQ; // Ask CAN1 to enter normal mode
    while(CAN1->MSR & CAN_MSR_INAK) { } // Wait for CAN1 to sync, exiting init mode

    return 1;
}

int cnCANSend(uint32_t id, unsigned len, const uint8_t data[len])
{
    if(!(CAN1->TSR & (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2)))
    {
        // All TX mailboxes are full, can't send message
        return -1;
    }

    uint32_t mailboxId = (CAN1->TSR & CAN_TSR_CODE) >> 24; // First empty mailbox, 0..2
    CAN1->OUTBOX[mailboxId].IR = id & ~CAN_TIR_TXRQ; // Set id, IDE and RTR; ensure TXRQ is 0 for now
    len = len <= 8 ? len : 8; // Max payload: 8 bytes - *truncate length accordingly*!
    CAN1->OUTBOX[mailboxId].DTR = len; // Set Data Length Code

    // Copy data to DLR and DHR
    volatile uint8_t *dest = (volatile uint8_t *)&CAN1->OUTBOX[mailboxId].DLR;
    for(unsigned i = 0; i < len; i ++)
    {
        *dest++ = *data++;
    }

    CAN1->OUTBOX[mailboxId].IR |= CAN_TIR_TXRQ; // Trigger transmission
    return (int)len;
}

int cnCANRecv(uint32_t *recvId, unsigned maxLen, uint8_t data[maxLen])
{
    // FIXME IMPLEMENT!
    return -1;
}
