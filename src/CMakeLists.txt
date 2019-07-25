# CANnuccia/src/CMakeLists.txt
#
# Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# NOTE: Use one of the provided toolchain files, they will define
#       `CMAKE_SYSTEM_PROCESSOR` and add #defines for core CN_* macros!
if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*stm32")
    set(CN_TARGET stm32)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES ".*avr")
    set(CN_TARGET avr)
else()
    message(FATAL_ERROR "Unknown target. Specify a CANnuccia toolchain file for CMake!")
endif()

add_subdirectory(${CN_TARGET}/)

add_executable(cn
    common/main.c
)
set_target_properties(cn PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    OUTPUT_NAME "cn"
    SUFFIX ".elf"
)
target_link_libraries(cn PUBLIC
    cn_${CN_TARGET}
)
