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
inline void mcpReadMulti(uint8_t addr, int n, uint8_t outValues[static n])
{
    spiSelect();
    spiTransfer(MCP_CMD_READ);
    spiTransfer(addr);
    for(int i = 0; i < n; i ++)
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
inline void mcpWriteMulti(uint8_t addr, int n, const uint8_t values[static n])
{
    spiSelect();
    spiTransfer(MCP_CMD_WRITE);
    spiTransfer(addr);
    for(int i = 0; i < n; i ++)
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

/// Writes the CAN extended identifier in `id` (most significant 29 bits) to
/// the group of four registers starting at `addr` in the MCP CAN controller.
///
/// The four registers starting from `addr` should be xSIDH, xSIDL, xEID8 and xEID0;
/// see MCP's datasheet.
inline void mcpWriteEID(uint32_t id, uint8_t addr)
{
    uint8_t regs[4];
    regs[0] = (id & 0x00003FC0UL) >> 6; // ID bits 3..10 -> SIDH bits 0..7
    regs[1] = MCP_RXFSIDL_EXIDE; // Set extended ID bit in SIDL
    regs[1] |= (id & 0xC0000000UL) >> 30; // ID bits 27, 28 -> SIDL bits 0, 1
    regs[1] |= (id & 0x00000038UL) << 2; // ID bits 0, 1, 2 -> SIDL bits 5, 6, 7
    regs[2] = (id & 0x003FC000UL) >> 14; // ID bits 11..18 -> EID0 bits 0..7
    regs[3] = (id & 0x3FC00000UL) >> 22; // ID bits 19..26 -> EID8 bits 0..7

    mcpWriteMulti(addr, sizeof(regs), regs);
}

/// Reads a CAN extended identifier (setting the most significant 29 bits of the
/// return value) from a group of four registers starting at `addr` in the MCP CAN controller.
///
/// The four registers starting from `addr` should be xSIDH, xSIDL, xEID8 and xEID0;
/// see MCP's datasheet.
inline uint32_t mcpReadEID(uint8_t addr)
{
    uint8_t regs[4];
    mcpReadMulti(addr, sizeof(regs), regs);

    uint32_t eid = 0;
    eid |= (uint32_t)(regs[0]) << 3; // SIDH bits 0..7 -> ID bits 3..10
    eid |= (uint32_t)(regs[1]) << 30; // SIDL BITS 0, 1 -> ID bits 27, 28
    eid |= (uint32_t)(regs[1]) >> 5; // SIDL bits 5, 6, 7 -> ID bits 0, 1, 2
    eid |= (uint32_t)(regs[2]) << 14; //EID0 bits 0..7 -> ID bits 11..18
    eid |= (uint32_t)(regs[3]) << 22; // EID8 bits 0..7 -> ID bits 19..26

    return eid;
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

    // Enter configuration mode
    mcpModify(MCP_REG_CANCTRL, MCP_MODEMASK, MCP_MODE_CONFIG);
    if((mcpRead(MCP_REG_CANCTRL) & MCP_MODEMASK) != MCP_MODE_CONFIG)
    {
        return 0; // Could not change mode
    }

    // Set CAN timings
    mcpWrite(MCP_REG_CNF1, MCP_CNF1_VAL);
    mcpWrite(MCP_REG_CNF2, MCP_CNF2_VAL);
    mcpWrite(MCP_REG_CNF3, MCP_CNF3_VAL);

    // Apply filter id & mask to MCP's CAN filter 0.
    mcpWriteEID(id, MCP_REG_RXF0SIDH);
    mcpWriteEID(mask, MCP_REG_RXM0SIDH);

    // Enter normal mode
    mcpModify(MCP_REG_CANCTRL, MCP_MODEMASK, MCP_MODE_NORMAL);
    if((mcpRead(MCP_REG_CANCTRL) & MCP_MODEMASK) != MCP_MODE_NORMAL)
    {
        return 0; // Could not change mode
    }

    return 1;
}


int cnCANInit(uint32_t id, uint32_t mask)
{
    // MOSI, SCK and CS as outputs; CS=hi
    SPI_PORT |= CS_PIN;
    SPI_DDR |= MOSI_PIN | SCK_PIN | CS_PIN;

    // SPI enabled in master mode, CPHA=0, CPOL=0, MSB first, frequency=fOSC/2
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR |= (1 << SPI2X);

    int mcpOk = mcpSetup(id, mask);
    return mcpOk;
}

int cnCANSend(uint32_t id, unsigned len, const uint8_t data[len])
{
    // FIXME IMPLEMENT!
    return -1;
}

int cnCANRecv(uint32_t *recvId, unsigned maxLen, uint8_t data[maxLen])
{
    // FIXME IMPLEMENT!
    return -1;
}
