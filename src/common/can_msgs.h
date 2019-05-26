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


/// The CAN filter mask to use in conjunction with `CN_CAN_TX_FILTER_ID`.
#define CN_CAN_TX_FILTER_MASK 0xFF000FFCu

/// The CAN filter used to match outgoing (master -> device) messages.
/// `cnCANDevMask()` a device id into this before use.
#define CN_CAN_TX_FILTER_ID 0xCA000004u

/// The CAN filter mask to use in conjunction with `CN_CAN_RX_FILTER_ID`.
#define CN_CAN_RX_FILTER_MASK 0xFF000FFCu

/// The CAN filter used to match ingoing (device -> master) messages.
/// `cnCANDevMask()` a device id into this before use.
#define CN_CAN_RX_FILTER_ID 0xCB000004u


/// Builds a CAN ID/mask by ORing a 8-bit device address (<< 4) into a 32-bit
/// base mask. Also sets the IDE bit (to mark a 29-bit filter, not a 11-bit one).
inline uint32_t cnCANDevMask(uint32_t mask, uint8_t devID)
{
    return ((uint32_t)mask | ((uint32_t)devID << 4) | 0x00000004u);
}

/// The CAN ID mask used to check if a message is of a certain type, i.e. if
/// `(msgId & CN_CAN_ID_MASK) == CN_CAN_MSG_x`
///
/// Masks the rightmost 20 bits; this way the device identifier, bit 3,
/// IDE (bit 2), RTR (bit 1) and TXRQ (bit 0) are ignored.
#define CN_CAN_MSGID_MASK 0xFFFFF000u


// IDs of a outgoing (master -> device) CAN message. See CANnuccia specs.
// `cnCANDevMask()` a device id into these before use. Note that the lowest 12
// bits are unset.
#define CN_CAN_MSG_PROG_REQ    0xCA001000u
#define CN_CAN_MSG_PROG_DONE   0xCA002000u
#define CN_CAN_MSG_UNLOCK      0xCA003000u
#define CN_CAN_MSG_ERASE_PAGES 0xCA004000u
#define CN_CAN_MSG_SEEK        0xCA005000u
#define CN_CAN_MSG_WRITE       0xCA006000u
#define CN_CAN_MSG_CHECK_PAGE  0xCA007000u

// IDs of an ingoing (device -> master) CAN message. See CANnuccia specs.
// `cnCANDevMask()` a device id into these before use. Note that the lowest 12
// bits are unset.
#define CN_CAN_MSG_PROG_REQ_RESP   0xCB001000u
#define CN_CAN_MSG_PROG_DONE_ACK   0xCB002000u
#define CN_CAN_MSG_UNLOCK_ACK      0xCB003000u
#define CN_CAN_MSG_TELL            0xCB005000u
#define CN_CAN_MSG_CHECK_PAGE_RESP 0xCB007000u


#endif // CAN_MSGS_H
