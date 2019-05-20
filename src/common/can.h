// CANnuccia/src/common/can.h - Interface to CAN hardware on different MCUs
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CAN_H
#define CAN_H

#include <stdint.h>

/// Initializes the CAN bus.
/// `id` and `mask` will be used to setup an ingoing message filter; CAN
/// messages will be read only if `messageId & mask == id & mask`
/// (for their lowest 29 bits).
/// Returns true on success or false on error.
int cnCANInit(uint32_t id, uint32_t mask);

/// Sends a CAN message.
/// The lowest 29 bits of `id` are used. `len` bytes of `data` are sent with the
/// message; if `len > 8`, only the first 8 bytes are sent.
/// Returns the number of bytes effectively sent, or a negative value on error.
int cnCANSend(uint32_t id, unsigned len, const uint8_t data[len]);

/// Polls for a received CAN message.
/// The lowest 29 bits of `*recvId` will be set to the id of the message, and
/// up to `maxLen` bytes of its payload will be copied to `data`.
/// Returns the number of bytes effectively read, or a negative value if no
/// message was received or if an error occurred.
int cnCANRecv(uint32_t *recvId, unsigned maxLen, uint8_t data[maxLen]);

#endif // CAN_H
