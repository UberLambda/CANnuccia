# CANnuccia/src/stm32/CMakeLists.txt
#
# Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(cn_stm32 STATIC
    startup.c
    flash.c
    can.c
    debug.c
    util.c
    timer.c
)
