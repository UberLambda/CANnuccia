// CANnuccia/src/avr/can.c - AVR (+ MCP25625) implementation of common/can.h
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "common/can.h"

#ifndef F_CPU
#   define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

// MCP25625's chip select, MOSI, SCK
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define CS_PIN 0x80
#define MOSI_PIN 0x08
#define SCK_PIN 0x20

/// Writes `byte` to SPI and returns the received response byte.
inline uint8_t spiTransfer(uint8_t byte)
{
    SPDR = byte;
    while(!(SPSR & (1 << SPIF))) { }
    return SPDR;
}

/// Puts the MCP chip select pin low.
inline void spiSelect(void)
{
    SPI_PORT &= ~CS_PIN;
}

/// Puts the MCP chip select pin high.
inline void spiDeselect(void)
{
    SPI_PORT |= CS_PIN;
}


// Register addresses and constants from MCP2565 (& MCP2510/MCP2515)'s datasheet
#define MCP_CMD_RESET 0xC0
#define MCP_CMD_READ 0x03
#define MCP_CMD_READ_RXBUF 0x90
#define MCP_CMD_WRITE 0x02
#define MCP_CMD_LOAD_TXBUF 0x40
#define MCP_CMD_RTS 0x80
#define MCP_CMD_READ_STATUS 0xA0
#define MCP_CMD_RX_STATUS 0xB0
#define MCP_CMD_BIT_MODIFY 0x05

#define MCP_REG_CANCTRL 0x0F // (or any 0x_F address)
#define MCP_REG_CNF1 0x2A
#define MCP_REG_CNF2 0x29
#define MCP_REG_CNF3 0x28
#define MCP_REG_RXF0SIDH 0x00
#define MCP_REG_RXM0SIDH 0x20

#define MCP_MODEMASK 0xE0
#define MCP_MODE_NORMAL 0x00
#define MCP_MODE_SLEEP 0x20
#define MCP_MODE_LOOPBACK 0x40
#define MCP_MODE_LISTEN_ONLY 0x60
#define MCP_MODE_CONFIG 0x80

#define MCP_RXFSIDL_EXIDE 0x08

#define MCP_BDLC_RTR 0x40

#define MCP_STATUS_TX0REQ 0x04
#define MCP_STATUS_TX1REQ 0x10
#define MCP_STATUS_TX2REQ 0x40

#define MCP_RXSTATUS_RXB0 0x40
#define MCP_RXSTATUS_RXB1 0x80

// Assume the AVR and the MCP run at the same clock speed.
// CAN speed = 1Mbps, sample point = 75%
// See: https://www.kvaser.com/support/calculators/bit-timing-calculator/
// TODO IMPLEMENT: User-configurable CAN speed, independent on AVR clock speed
//                 (maybe set via a CMake variable?)
#if F_CPU == 16000000UL
#   define MCP_CNF1_VAL 0x00
#   define MCP_CNF2_VAL 0x91
#   define MCP_CNF3_VAL 0x01
#else
#   error "CPU frequency expected to be 16MHz!"
#endif

#define MCP_

/// Reads a register of the MCP CAN controller.
inline uint8_t mcpRead(uint8_t addr)
{
    spiSelect();
    spiTransfer(MCP_CMD_READ);
    spiTransfer(addr);
    uint8_t reg = spiTransfer(0x00);
    spiDeselect();
    return reg;
}

/// Reads `n` registers of the MCP CAN controller, starting from the one at `addr`.
inline void mcpReadMulti(uint8_t addr, unsigned n, uint8_t outValues[static n])
{
    spiSelect();
    spiTransfer(MCP_CMD_READ);
    spiTransfer(addr);
    for(unsigned i = 0; i < n; i ++)
    {
        outValues[i] = spiTransfer(0x00);
    }
    spiDeselect();
}

/// Writes to a register in the MCP CAN controller.
inline void mcpWrite(uint8_t addr, uint8_t value)
{
    spiSelect();
    spiTransfer(MCP_CMD_WRITE);
    spiTransfer(addr);
    spiTransfer(value);
    spiDeselect();
}

/// Write to `n` registers in the MCP CAN controller, starting from the one at
/// `addr`.
inline void mcpWriteMulti(uint8_t addr, unsigned n, const uint8_t values[static n])
{
    spiSelect();
    spiTransfer(MCP_CMD_WRITE);
    spiTransfer(addr);
    for(unsigned i = 0; i < n; i ++)
    {
        spiTransfer(values[i]);
    }
    spiDeselect();
}

/// Modifies specific bits of the register at `addr` in the MCP CAN controller.
inline void mcpModify(uint8_t addr, uint8_t mask, uint8_t value)
{
    spiSelect();
    spiTransfer(MCP_CMD_BIT_MODIFY);
    spiTransfer(addr);
    spiTransfer(mask);
    spiTransfer(value);
    spiDeselect();
}

/// Writes the CAN extended identifier in `id` (most significant 29 bits) in the
/// format of the four xSIDH, xSIDL, xEID8 and xEID0 in the MCP CAN controller.
inline void mcpPutEID(uint32_t id, uint8_t outRegs[static 4])
{
    outRegs[0] = (id & 0x00003FC0UL) >> 6; // ID bits 3..10 -> SIDH bits 0..7
    outRegs[1] = MCP_RXFSIDL_EXIDE; // Set extended ID bit in SIDL
    outRegs[1] |= (id & 0xC0000000UL) >> 30; // ID bits 27, 28 -> SIDL bits 0, 1
    outRegs[1] |= (id & 0x00000038UL) << 2; // ID bits 0, 1, 2 -> SIDL bits 5, 6, 7
    outRegs[2] = (id & 0x003FC000UL) >> 14; // ID bits 11..18 -> EID0 bits 0..7
    outRegs[3] = (id & 0x3FC00000UL) >> 22; // ID bits 19..26 -> EID8 bits 0..7
}

/// Reads a CAN extended identifier (setting the most significant 29 bits of the
/// return value) that have been read to `regs` in the format of the MCP CAN
/// controller: xSIDH, xSIDL, xEID8 and xEID0.
inline uint32_t mcpGetEID(const uint8_t regs[static 4])
{
    uint32_t eid = 0;
    eid |= (uint32_t)(regs[0]) << 3; // SIDH bits 0..7 -> ID bits 3..10
    eid |= (uint32_t)(regs[1]) << 30; // SIDL BITS 0, 1 -> ID bits 27, 28
    eid |= (uint32_t)(regs[1]) >> 5; // SIDL bits 5, 6, 7 -> ID bits 0, 1, 2
    eid |= (uint32_t)(regs[2]) << 14; //EID0 bits 0..7 -> ID bits 11..18
    eid |= (uint32_t)(regs[3]) << 22; // EID8 bits 0..7 -> ID bits 19..26
    return eid;
}

/// Changes the mode of the MCP CAN controller to a different `MCP_MODE_*`.
/// Returns true if the change happened successfully or false otherwise.
inline int mcpChangeMode(uint8_t newMode)
{
    mcpModify(MCP_REG_CANCTRL, MCP_MODEMASK, newMode);
    return (mcpRead(MCP_REG_CANCTRL) & MCP_MODEMASK) == newMode;
}

/// Writes the given id and mask pair to MCP CAN's filter 0.
static void mcpSetFilter0(uint32_t id, uint32_t mask)
{
    uint8_t regs[4];
    mcpPutEID(id, regs);
    mcpWriteMulti(MCP_REG_RXF0SIDH, sizeof(regs), regs);
    mcpPutEID(mask, regs);
    mcpWriteMulti(MCP_REG_RXM0SIDH, sizeof(regs), regs);
}

/// Initializes the MCP CAN controller attached to SPI,
/// setting its baud rate and filter.
/// Returns true on success or false on error.
static int mcpSetup(uint32_t id, uint32_t mask)
{
    // Reset the CAN chip and wait a bit until it restarts
    spiSelect();
    spiTransfer(MCP_CMD_RESET);
    spiDeselect();
    _delay_ms(1);

    if(!mcpChangeMode(MCP_MODE_CONFIG))
    {
        return 0;
    }

    // Set CAN timings
    mcpWrite(MCP_REG_CNF1, MCP_CNF1_VAL);
    mcpWrite(MCP_REG_CNF2, MCP_CNF2_VAL);
    mcpWrite(MCP_REG_CNF3, MCP_CNF3_VAL);

    // Apply filter id & mask to MCP's CAN filter 0.
    mcpSetFilter0(id, mask);

    // NOTE: RXB0CTRL should be 0x00 after the reset that was issued - i.e. set
    // to use filter 0 to filter ingoing messages, no rollover, no RTR

    return mcpChangeMode(MCP_MODE_NORMAL);
}


static int inited = 0;

int cnCANInit(uint32_t id, uint32_t mask)
{
    if(!inited)
    {
        // MOSI, SCK and CS as outputs; CS=hi
        SPI_PORT |= CS_PIN;
        SPI_DDR |= MOSI_PIN | SCK_PIN | CS_PIN;

        // SPI enabled in master mode, CPHA=0, CPOL=0, MSB first, frequency=fOSC/2
        SPCR = (1 << SPE) | (1 << MSTR);
        SPSR |= (1 << SPI2X);

        inited = mcpSetup(id, mask);
        return inited;
    }
    else
    {
        // Just change filters
        if(!mcpChangeMode(MCP_MODE_CONFIG))
        {
            return 0;
        }
        mcpSetFilter0(id, mask);
        return mcpChangeMode(MCP_MODE_NORMAL);
    }
}

int cnCANSend(uint32_t id, unsigned len, const uint8_t data[len])
{
    len = len > 8 ? len : 8; // Cap length to maximum

    // Check if any transmission mailbox is free
    spiSelect();
    spiTransfer(MCP_CMD_READ_STATUS);
    uint8_t status = spiTransfer(0x00);
    spiDeselect();

    uint8_t mailboxId;
    if(!(status & MCP_STATUS_TX0REQ))
    {
        mailboxId = 0;
    }
    else if(!(status & MCP_STATUS_TX1REQ))
    {
        mailboxId = 1;
    }
    else if(!(status & MCP_STATUS_TX2REQ))
    {
        mailboxId = 2;
    }
    else
    {
        // No TX mailbox free = can't send any message
        return -1;
    }

    uint8_t buffer[13];

    // buffer[0..3] = TXB_SIDH, SIDL, EID8, EID0
    mcpPutEID(id, buffer);

    // buffer[4] = DLC
    buffer[4] = (uint8_t)len;
    if(id & CN_CAN_RTR)
    {
        buffer[4] |= MCP_BDLC_RTR;
    }

    // buffer[5..12] = payload
    for(unsigned i = 0; i < len; i ++)
    {
        buffer[5 + i] = data[i];
    }

    // Write destination id, data length code and data all in one go
    spiSelect();
    spiTransfer(MCP_CMD_LOAD_TXBUF | (uint8_t)(mailboxId << 1));
    for(unsigned i = 0; i < sizeof(buffer); i ++)
    {
        spiTransfer(buffer[i]);
    }
    spiDeselect();

    // Request the written-to mailbox to be sent out
    spiSelect();
    spiTransfer(MCP_CMD_RTS | (uint8_t)(0x01 << mailboxId));
    spiDeselect();

    return (int)len;
}

int cnCANRecv(uint32_t *recvId, unsigned maxLen, uint8_t data[maxLen])
{
    // FIXME IMPLEMENT!
    return -1;
}
