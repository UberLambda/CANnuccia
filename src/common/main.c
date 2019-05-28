// CANnuccia/src/common/main.c - Entry point of CANnuccia (for devices)
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stddef.h>
#include "common/debug.h"
#include "common/can.h"
#include "common/can_msgs.h"
#include "common/flash.h"
#include "common/util.h"

/// Set to false when the bootloader is to exit its main loop
volatile static int running = 0;

/// Set to true when the flash is unlocked for writing
volatile static int unlocked = 0;

/// Points to the first byte in flash of the selected page
volatile static uintptr_t selPageAddr = 0;

/// WRITE head byte offset into the selected page
volatile static uintptr_t selWriteOffset = 0;

/// Page-sized buffer where WRITEs for the selected page are stored
static uint8_t selPageWrites[CN_FLASH_PAGE_SIZE];

int main(void)
{
    // Only listen to CAN messages from master to this device
    uint8_t devId = 0xFE; // FIXME READ REAL ADDRESS FROM OPTION BYTES/EEPROM
    uint32_t txFilterId = cnCANDevMask(CN_CAN_TX_FILTER_ID, devId);
    cnCANInit(txFilterId, CN_CAN_TX_FILTER_MASK);

    cnDebugInit();
    cnDebugLed(1);

    // FIXME IMPLEMENT: Set a timer interrupt to ~3 seconds from now; if no
    //                  PROG_REQ has arrived by then stop listening to CAN and
    //                  jump to the user program

    // CAN message pump (main loop)
    // See the CANnuccia specs for what each message is supposed to do
    uint32_t inMsgId, outMsgId;
    uint8_t inMsgData[8], outMsgData[8];
    int inMsgDataLen;
    uint16_t selPageWritesCRC = 0; // CRC of the WRITEs in `selPageWrites`

    running = 1;
    while(running)
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
            // FIXME IMPLEMENT: Master has asked to program us; stop the 3-second
            //                  timeout timer and ACK the request - sending data
            //                  about us
            break;

        case CN_CAN_MSG_UNLOCK:
            unlocked = cnFlashUnlock();
            if(unlocked)
            {
                outMsgId = cnCANDevMask(CN_CAN_MSG_UNLOCKED, devId);
                cnCANSend(outMsgId, 0, NULL);
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
                    selPageAddr = newPageAddr;

                    outMsgId = cnCANDevMask(CN_CAN_MSG_PAGE_SELECTED, devId);
                    cnWriteU32LE(outMsgData, selPageAddr); // (send the PAGE_MASKed-out address)
                    cnCANSend(outMsgId, 4, outMsgData);
                }
            }
            break;

        case CN_CAN_MSG_SEEK:
            if(inMsgDataLen == 4)
            {
                uint32_t newOffset = cnReadU32LE(inMsgData);
                if(newOffset < sizeof(selPageWrites))
                {
                    selWriteOffset = newOffset;
                }
            }
            break;

        case CN_CAN_MSG_WRITE:
            for(int i = 0;
                i < inMsgDataLen && selWriteOffset < sizeof(selPageWrites);
                i ++)
            {
                selPageWrites[selWriteOffset] = inMsgData[i];
                selWriteOffset ++;
            }
            break;

        case CN_CAN_MSG_CHECK_WRITES:
            selPageWritesCRC = cnCRC16(sizeof(selPageWrites), selPageWrites);

            outMsgId = cnCANDevMask(CN_CAN_MSG_WRITES_CHECKED, devId);
            cnWriteU16LE(outMsgData, selPageWritesCRC);
            cnCANSend(outMsgId, 2, outMsgData);
            break;

        case CN_CAN_MSG_COMMIT_WRITES:
            if(!unlocked)
            {
                // Can't commit writes if the flash is still locked
                continue;
            }
            cnFlashBeginWrite(selPageAddr);
            cnFlashFill(0, sizeof(selPageWrites), selPageWrites);
            cnFlashEndWrite();

            outMsgId = cnCANDevMask(CN_CAN_MSG_WRITES_COMMITTED, devId);
            cnWriteU32LE(outMsgData, selPageAddr);
            cnCANSend(outMsgId, 4, outMsgData);
            break;

        case CN_CAN_MSG_PROG_DONE:
            outMsgId = cnCANDevMask(CN_CAN_MSG_PROG_DONE_ACK, devId);
            cnCANSend(outMsgId, 0, NULL);
            running = 0;
            break;
        }
    }

    cnDebugLed(0);

    // At this point we've either been issued a `PROG_DONE` msg or the bootloader
    // timed out; in both cases the bootloader is done running!
    cnFlashLock();
    cnJumpToProgram();
}
