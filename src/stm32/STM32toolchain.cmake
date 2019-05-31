# CANnuccia/src/stm32/STM32toolchain.cmake - CMake toolchain for arm-none-eabi
#
# Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(ARM_CPU "cortex-m3" CACHE STRING "The type of ARM core to compile for")
set(ARM_PREFIX "arm-none-eabi" CACHE STRING "The prefix of the ARM crosscompiler toolchain")
set(STM32_PART "stm32f103c8" CACHE STRING "The STM32 chip's part name")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_C_COMPILER "${ARM_PREFIX}-gcc")
set(CMAKE_C_COMPILER_TARGET "${ARM_PREFIX}")
set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_C_COMPILER_FORCED YES)
set(CMAKE_CXX_COMPILER "${ARM_PREFIX}-g++")
set(CMAKE_CXX_COMPILER_TARGET "${ARM_PREFIX}")
set(CMAKE_CXX_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_FORCED YES)

set(CMAKE_C_FLAGS_DEBUG "-g -Og -mthumb -mcpu=${ARM_CPU} -fstrict-volatile-bitfields")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "-Os -mthumb -mcpu=${ARM_CPU} -fstrict-volatile-bitfields")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

# SELF_DIR: the directory where this toolchain file resides
get_filename_component(SELF_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)

set(CMAKE_EXE_LINKER_FLAGS_LIST
    -mthumb
    -mcpu=${ARM_CPU}
    "-T${SELF_DIR}/ld/${STM32_PART}.ld"
    -nostdlib
    -nostartfiles
)
string(REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_LIST}")

# #define core macros required to build CANnuccia
# FIXME: High-density (> 128kB) devices have 2kB pages, not 1kB
# FIXME: This should likely be moved out of the toolchain file to somewhere better!
add_definitions(
    -DCN_FLASH_PAGE_SIZE=0x400u # 1kB pages
    -DCN_FLASH_PAGE_MASK=0xFFFFFC00u
    -DCN_FLASH_BOOTLOADER_SIZE=0x2000u # 4kB reserved to CANnuccia
    -DCN_PLATFORM_IS_STM32=1
)
