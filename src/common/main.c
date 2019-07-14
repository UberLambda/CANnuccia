// CANnuccia/src/common/main.c - Entry point of CANnuccia (for devices)
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stddef.h>
#include "common/util.h"
#include "common/can.h"
#include "common/can_msgs.h"
#include "common/flash.h"
#include "common/timer.h"
#include "common/debug.h"

/// The current state of the bootloader.
volatile static enum
{
    IDLE, ///< Timeout engaged, waiting for a PROG_REQ.
    LOCKED, ///< Got a PROG_REQ, waiting for an UNLOCK.
    UNLOCKED, ///< Flash unlocked, can COMMIT_WRITES to it.
    DONE, ///< Bootloader done, message pump is to quit.

} state = IDLE;

/// Data about the currently-selected page.
static struct Page
{
    volatile uintptr_t addr; ///< Points to the first byte in flash of the page.
    volatile uintptr_t writeOffset; ///< WRITE head byte offset into the selected page.
    uint8_t writes[CN_FLASH_PAGE_SIZE]; ///< All WRITEs to be committed to the selected page.

} selPage = {0};

/// The timeout in microseconds after which to the bootloader stops listening
/// for CAN messages
#define BOOTLOADER_TIMEOUT_US 3000000


/// Executed when the bootloader times out, exits the CAN message pump.
static void onTimeout(void)
{
    state = DONE;
}

int main(void)
{
    cnDebugInit();
    cnDebugLed(1);

    // Only listen to CAN messages from master to this device
    uint8_t devId = cnReadDevId();
    uint32_t txFilterId = cnCANDevMask(CN_CAN_TX_FILTER_ID, devId);
    cnCANInit(txFilterId, CN_CAN_TX_FILTER_MASK);

    // Set the bootloader timeout: if no PROG_REQ has arrived by that time, stop
    // the CAN message pump and jump to the user program.
    cnTimerStart(BOOTLOADER_TIMEOUT_US, 1, onTimeout);

    // CAN message pump (main loop)
    // See the CANnuccia specs for what each message is supposed to do
    uint32_t inMsgId, outMsgId;
    uint8_t inMsgData[8], outMsgData[8];
    int inMsgDataLen;
    uint16_t selPageWritesCRC = 0; // CRC of the WRITEs in `selPageWrites`

    state = IDLE;
    while(state != DONE)
    {
        inMsgDataLen = cnCANRecv(&inMsgId, sizeof(inMsgData), inMsgData);
        if(inMsgDataLen < 0)
        {
            // No message from master to us
            continue;
        }

        switch(inMsgId & CN_CAN_MSGID_MASK)
        {
        case CN_CAN_MSG_PROG_REQ:
            if(state == IDLE)
            {
                cnTimerStop();
                state = LOCKED;
            }
            // Always answer with stats after a PROG_REQ (even if we already were not IDLE):
            // 1. log2(size of a flash page): U8
            // 2. Total number of flash pages: U16
            // 3. ELF machine type (e_machine): U16
            outMsgId = cnCANDevMask(CN_CAN_MSG_PROG_REQ_RESP, devId);
            outMsgData[0] = (uint8_t)cnLog2I(CN_FLASH_PAGE_SIZE);
            cnWriteU16LE(outMsgData + 1, (uint16_t)(cnFlashSize() / CN_FLASH_PAGE_SIZE));
            cnWriteU16LE(outMsgData + 3, CN_E_MACHINE);
            cnCANSend(outMsgId, 5, outMsgData);
            break;

        case CN_CAN_MSG_UNLOCK:
            if(state == IDLE)
            {
                int unlocked = cnFlashUnlock();
                if(unlocked)
                {
                    state = UNLOCKED;
                    outMsgId = cnCANDevMask(CN_CAN_MSG_UNLOCKED, devId);
                    cnCANSend(outMsgId, 0, NULL);
                }
            }
            break;

        case CN_CAN_MSG_SELECT_PAGE:
            if(inMsgDataLen == 4)
            {
                uint32_t newPageAddr = cnReadU32LE(inMsgData);
                // Truncate the given address, making sure that it points to the
                // very first byte of the page and not to some other position in
                // it
                newPageAddr &= CN_FLASH_PAGE_MASK;

                if(cnFlashPageWriteable(newPageAddr))
                {
                    selPage.addr = newPageAddr;

                    outMsgId = cnCANDevMask(CN_CAN_MSG_PAGE_SELECTED, devId);
                    cnWriteU32LE(outMsgData, selPage.addr); // (send the PAGE_MASKed-out address)
                    cnCANSend(outMsgId, 4, outMsgData);
                }
            }
            break;

        case CN_CAN_MSG_SEEK:
            if(inMsgDataLen == 4)
            {
                uint32_t newOffset = cnReadU32LE(inMsgData);
                if(newOffset < sizeof(selPage.writes))
                {
                    selPage.writeOffset = newOffset;
                }
            }
            break;

        case CN_CAN_MSG_WRITE:
            for(int i = 0;
                i < inMsgDataLen && selPage.writeOffset < sizeof(selPage.writes);
                i ++)
            {
                selPage.writes[selPage.writeOffset] = inMsgData[i];
                selPage.writeOffset ++;
            }
            break;

        case CN_CAN_MSG_CHECK_WRITES:
            selPageWritesCRC = cnCRC16(sizeof(selPage.writes), selPage.writes);

            outMsgId = cnCANDevMask(CN_CAN_MSG_WRITES_CHECKED, devId);
            cnWriteU16LE(outMsgData, selPageWritesCRC);
            cnCANSend(outMsgId, 2, outMsgData);
            break;

        case CN_CAN_MSG_COMMIT_WRITES:
            if(state == UNLOCKED)
            {
                if(!cnFlashBeginWrite(selPage.addr))
                {
                    break;
                }
                cnFlashFill(0, sizeof(selPage.writes), selPage.writes);
                if(!cnFlashEndWrite())
                {
                    break;
                }

                outMsgId = cnCANDevMask(CN_CAN_MSG_WRITES_COMMITTED, devId);
                cnWriteU32LE(outMsgData, selPage.addr);
                cnCANSend(outMsgId, 4, outMsgData);
            }
            break;

        case CN_CAN_MSG_PROG_DONE:
            outMsgId = cnCANDevMask(CN_CAN_MSG_PROG_DONE_ACK, devId);
            cnCANSend(outMsgId, 0, NULL);
            state = DONE;
            break;
        }
    }

    cnDebugLed(0);

    // At this point we've either been issued a `PROG_DONE` msg or the bootloader
    // timed out; in both cases the bootloader is done running!
    cnFlashLock();
    cnJumpToProgram();
}
