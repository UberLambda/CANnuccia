// CANnuccia/src/common/debug.h - The interface to debug utils on target MCUs
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DEBUG_H
#define DEBUG_H

/// Initializes debug functionalities on the target MCU.
/// Returns true on success or false on error.
int cnDebugInit(void);

/// Turns the debug LED on or off.
/// `cnDebugInit()` must have been called beforehand.
void cnDebugLed(int on);

#endif // DEBUG_H
