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
    struct CanFilter FILTER[28];
};
#define CAN1 ((volatile struct Can *)0x40006400)
#define CAN_BTR_LBKM 0x40000000u
#define CAN_TIR_TXRQ 0x00000001u
#define CAN_MCR_ABOM 0x00000040u
#define CAN_MCR_AWUM 0x00000020u
#define CAN_MCR_SLEEP 0x00000002u
#define CAN_MCR_INRQ 0x00000001u
#define CAN_MSR_SLAK 0x00000002u
#define CAN_MSR_INAK 0x00000001u
#define CAN_TSR_TME2 0x10000000u
#define CAN_TSR_TME1 0x08000000u
#define CAN_TSR_TME0 0x04000000u
#define CAN_TSR_CODE 0x03000000u
#define CAN_FMR_FINIT 0x00000001u
#define CAN_DTR_DLC 0x0000000Fu
#define CAN_RFR_RFOM 0x00000020u
#define CAN_RFR_FOVR 0x00000010u
#define CAN_RFR_FULL 0x00000008u
#define CAN_RFR_FMP 0x00000003u


const unsigned CN_CAN_RATE = 100000; // (100kbps, matches BTR's value)


/// Sets the CAN1 filter number `n` to the given 32-bit id & mask pair.
inline static void initCANFilter(unsigned n, uint32_t id, uint32_t mask)
{
    const uint32_t fltBit = (1u << n);
    CAN1->FM1R &= ~fltBit; // CAN1: filter n in mask mode
    CAN1->FS1R |= fltBit; // CAN1: filter n is 32-bit (not 16-bit)
    CAN1->FFA1R &= ~fltBit; // CAN1: filter n assigned to FIFO 0
    CAN1->FILTER[n].R1 = id;
    CAN1->FILTER[n].R2 = mask;
    CAN1->FA1R |= fltBit; // CAN1: filter 0 is active
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
    // TODO: set other CAN options if needed (NART, RFLM, TXFP...)

    // Set BTR here to change the CAN baud rate; optionally set CAN_BTR_LBKM to
    // enable loopback for debugging. Assumes a 72MHz clock & target CAN rate
    // matching `CN_CAN_RATE` as defined above.
    // -> http://www.bittiming.can-wiki.info/?CLK=72&ctype=bxCAN&SamplePoint=87.5&SJW=1&calc=1 <-
    CAN1->BTR = 0x001C002Cu;

    CAN1->MCR &= ~CAN_MCR_INRQ; // Ask CAN1 to enter normal mode
    while(CAN1->MSR & CAN_MSR_INAK) { } // Wait for CAN1 to exiting init mode
    CAN1->MCR &= ~CAN_MCR_SLEEP; // Wake CAN1 from sleep. It should now sync...

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
    len = len <= 8 ? len : 8; // *Truncate length to 8*!
    CAN1->OUTBOX[mailboxId].DTR = len & CAN_DTR_DLC; // Set Data Length Code

    // Copy data to DLR and DHR
    // WARNING: The ARM core does NOT support unaligned access; copying byte
    //          arrays directly will corrupt the data - need the ugly switch
    //          below or a `memcpy()`!
    volatile uint32_t *DHR = &CAN1->OUTBOX[mailboxId].DHR, *DLR = &CAN1->OUTBOX[mailboxId].DLR;
    *DHR = 0;
    *DLR = 0;
    switch(len)
    {
    case 8:
    default:
        *DHR |= ((uint32_t)data[7] << 24);
    case 7:
        *DHR |= ((uint32_t)data[6] << 16);
    case 6:
        *DHR |= ((uint32_t)data[5] << 8);
    case 5:
        *DHR |= data[4];
    case 4:
        *DLR |= ((uint32_t)data[3] << 24);
    case 3:
        *DLR |= ((uint32_t)data[2] << 16);
    case 2:
        *DLR |= ((uint32_t)data[1] << 8);
    case 1:
        *DLR |= data[0];
    case 0:
        break;
    }

    CAN1->OUTBOX[mailboxId].IR |= CAN_TIR_TXRQ; // Trigger transmission
    return (int)len;
}

int cnCANRecv(uint32_t *recvId, unsigned maxLen, uint8_t data[maxLen])
{
    // assert(recvId);
    // NOTE: Code always reads FIFO 0 - TODO: make it work also for FIFO 1

    if(!(CAN1->RF0R & CAN_RFR_FMP))
    {
        // No messages pending on FIFO 0
        return -1;
    }

    *recvId = CAN1->INBOX[0].IR; // Fetch FIFO 0 (CAN id, IDE, RTR)
    unsigned payloadLen = CAN1->INBOX[0].DTR & CAN_DTR_DLC; // Fetch FIFO 0 payload length
    maxLen = maxLen < payloadLen ? maxLen : payloadLen; // Truncate payload length to `maxLen`

    // Copy data out of FIFO 0's DLR and DHR
    // WARNING: The ARM core does NOT support unaligned access; copying byte
    //          arrays directly will corrupt the data - need the ugly switch
    //          below or a `memcpy()`!
    volatile const uint32_t *DHR = &CAN1->INBOX[0].DHR, *DLR = &CAN1->INBOX[0].DLR;
    switch(maxLen)
    {
    case 8:
    default:
        data[7] = (uint8_t)((*DHR & 0xFF000000u) >> 24);
    case 7:
        data[6] = (uint8_t)((*DHR & 0x00FF0000u) >> 16);
    case 6:
        data[5] = (uint8_t)((*DHR & 0x0000FF00u) >> 8);
    case 5:
        data[4] = (uint8_t)(*DHR & 0x000000FFu);
    case 4:
        data[3] = (uint8_t)((*DLR & 0xFF000000u) >> 24);
    case 3:
        data[2] = (uint8_t)((*DLR & 0x00FF0000u) >> 16);
    case 2:
        data[1] = (uint8_t)((*DLR & 0x0000FF00u) >> 8);
    case 1:
        data[0] = (uint8_t)(*DLR & 0x000000FFu);
    case 0:
        break;
    }

    // Message processed, clear it from FIFO 0
    CAN1->RF0R |= CAN_RFR_RFOM;

    return (int)maxLen;
}
