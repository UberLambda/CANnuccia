# CANnuccia/src/avr/AVRtoolchain.cmake - CMake toolchain for AVR (ATMega, with avr-libc)
#
# Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(AVR_PREFIX "avr" CACHE STRING "The prefix of the AVR crosscompiler toolchain")
set(AVR_PART "atmega328p" CACHE STRING "The AVR microcontroller's part name")

# 4kB bootloader is the maximum possible for ATMega328p (BOOTSZ=00).
# With BOOTSZ=00, the whole NRWW section is occupied by the bootloader.
# TODO: May differ for other AVR chips!
set(AVR_FLASH_SIZE 32768 CACHE STRING "The total size of program flash, in bytes")
set(AVR_BOOTLOADER_SIZE 4096 CACHE STRING "The size allocated to the bootloader section (via BOOTSZ), in bytes")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

set(CMAKE_C_COMPILER "${AVR_PREFIX}-gcc")
set(CMAKE_C_COMPILER_TARGET "${AVR_PREFIX}")
set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_C_COMPILER_FORCED YES)
set(CMAKE_CXX_COMPILER "${AVR_PREFIX}-g++")
set(CMAKE_CXX_COMPILER_TARGET "${AVR_PREFIX}")
set(CMAKE_CXX_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_FORCED YES)

set(CMAKE_C_FLAGS_DEBUG "-g -Og -mmcu=${AVR_PART} -flto -fstrict-volatile-bitfields")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "-Os -mmcu=${AVR_PART} -flto -fstrict-volatile-bitfields")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")


# Bootloaders on AVR are at the end of flash
math(EXPR BOOTLOADER_START_ADDR
    "${AVR_FLASH_SIZE} - ${AVR_BOOTLOADER_SIZE}"
    OUTPUT_FORMAT HEXADECIMAL
)
if((BOOTLOADER_START_ADDR LESS 0) OR (BOOTLOADER_START_ADDR GREATER_EQUAL ${AVR_FLASH_SIZE}))
    message(FATAL_ERROR "Calculated bootloader start address out of range!")
endif()

set(CMAKE_EXE_LINKER_FLAGS_LIST
    -flto
    -Wl,--section-start=.text=${BOOTLOADER_START_ADDR} # Relocate bootloader code
)
string(REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_LIST}")

# #define core macros required to build CANnuccia
# FIXME: Do all AVRs have 128B pages?
# FIXME IMPORTANT: Make sure the bootloader actually fits in CN_FLASH_BOOTLOADER_SIZE!!
# FIXME: This should likely be moved out of the toolchain file to somewhere better!
add_definitions(
    -DCN_FLASH_PAGE_SIZE=0x80u # 128B pages
    -DCN_FLASH_PAGE_MASK=0xFF80u
    -DCN_FLASH_BOOTLOADER_SIZE=${AVR_BOOTLOADER_SIZE}u # ${AVR_BOOTLOADER_SIZE} reserved to CANnuccia
    -DCN_E_MACHINE=0x0053u # AVR
    -DCN_PLATFORM_IS_AVR=1
)
