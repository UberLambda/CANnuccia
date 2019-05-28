// CANnuccia/src/common/timer.h - The interface to simple timers on target MCUs
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/// An ISR executed when a timer times out.
typedef void(*CNtimeoutFunc)(void);

/// Sets up a timer that, when `delayUs` microseconds have passed, interrupts
/// the program and starts `onTimeout()`.
/// It will trigger once if `oneshot`, otherwise it will keep triggering until
/// the timer is stopped via `cnTimerStop()`.
/// Returns true if the timer was setup successfully or false on error.
///
/// Repeated calls to this function will reset the timer and apply the new
/// parameters.
int cnTimerStart(uint32_t delayUs, int oneshot, CNtimeoutFunc onTimeout);

/// Stops the timer started via `cnTimerStart()`.
/// Does nothing if the timer is not ticking.
///
/// This aborts any pending calls to the timeout function.
void cnTimerStop(void);

#endif // TIMER_H
