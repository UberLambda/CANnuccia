# CANnuccia/src/CMakeLists.txt
#
# Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(cnSTM32 STATIC
    stm32/startup.c
    stm32/flash.c
    stm32/can.c
    stm32/debug.c
    stm32/util.c
    stm32/timer.c
)

add_executable(cn
    common/main.c
)
set_target_properties(cn PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    OUTPUT_NAME "cn"
    SUFFIX ".elf"
)
target_link_libraries(cn PUBLIC
    cnSTM32
)
