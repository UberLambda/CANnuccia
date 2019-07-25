// CANnuccia/src/common/cc.h - C-compiler-specific utility macros
//
// Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CC_H
#define CC_H

/// Marks that a function will never return.
#define CN_NORETURN __attribute__((noreturn))

/// Makes a function/variable reside in section `sec`.
#define CN_SECTION(sec) __attribute__((section(sec)))

/// Makes a function argument as unused.
#define CN_UNUSED(arg) ((void)arg)

#endif // CC_H
