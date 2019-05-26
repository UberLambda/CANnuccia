// CANnuccia/src/common/can_msgs.h - CANnuccia CAN message enumerations
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CAN_MSGS_H
#define CAN_MSGS_H

#include <stdint.h>

/// Builds a CAN ID/mask by ORing a 8-bit device address (<< 4) into a 32-bit
/// base mask. Also sets the IDE bit (to mark a 29-bit filter, not a 11-bit one).
#define CN_CAN_DEVMASK(mask, devID) ((uint32_t)mask | ((uint8_t)devID << 4) | 0x00000004)


/// The CAN filter mask to use in conjunction with `CN_CAN_TX_FILTER_ID`.
#define CN_CAN_TX_FILTER_MASK 0xFF000FFC

/// The CAN filter used to match outgoing (master -> device) messages.
/// `CN_CAN_DEVMASK()` a device id into this before use.
#define CN_CAN_TX_FILTER_ID 0xCA000004

/// The CAN filter mask to use in conjunction with `CN_CAN_RX_FILTER_ID`.
#define CN_CAN_RX_FILTER_MASK 0xFF000FFC

/// The CAN filter used to match ingoing (device -> master) messages.
/// `CN_CAN_DEVMASK()` a device id into this before use.
#define CN_CAN_RX_FILTER_ID 0xCB000004

/// The ID of a outgoing (master -> device) CAN message. See CANnuccia specs.
/// `CN_CAN_DEVMASK()` a device id into these before use.
typedef enum
{
    CN_CAN_MSG_PROG_REQ = 0xCA001000,
    CN_CAN_MSG_PROG_DONE = 0xCA002000,
    CN_CAN_MSG_UNLOCK = 0xCA003000,
    CN_CAN_MSG_ERASE_PAGES = 0xCA004000,
    CN_CAN_MSG_SEEK = 0xCA005000,
    CN_CAN_MSG_WRITE = 0xCA006000,
    CN_CAN_MSG_CHECK_PAGE = 0xCA007000,

} CNcanTXMsg;

/// The ID of an ingoing (device -> master) CAN message. See CANnuccia specs.
/// `CN_CAN_DEVMASK()` a device id into these before use.
typedef enum
{
    CN_CAN_MSG_PROG_REQ_RESP = 0xCB001000,
    CN_CAN_MSG_PROG_DONE_ACK = 0xCB002000,
    CN_CAN_MSG_TELL = 0xCB005000,
    CN_CAN_MSG_CHECK_PAGE_RESP = 0xCB007000,

} CNcanRXMsg;

#endif // CAN_MSGS_H
