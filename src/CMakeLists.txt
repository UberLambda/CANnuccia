# CANnuccia/src/CMakeLists.txt
#
# Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_executable(cn
    common/main.c
    stm32/startup.c
)

set_target_properties(cn PROPERTIES
    LINKER_LANGUAGE C
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
)
target_include_directories(cn PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
