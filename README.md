# CANnuccia
A bootloader for in-field programming of MCUs via CAN bus.  
See [CANale](https://github.com/UberLambda/CANale) for a tool to upload firmware to CANnuccia devices.

## Supported hardware
- STM32 microcontrollers with CAN via bxCAN.  
  Tested on the generic STM32F103C8T6 (ARM Cortex-M3) "blue pill" boards.
- AVR microcontrollers with CAN via [MCP25x CAN controllers](https://www.microchip.com/wwwproducts/en/en010406).  
  Tested on ATMega328p paired with MCP25625.

## Usage
Devices on a CANnuccia network have an 8-bit identifier (stored in the Data0 option byte on STM32 and on byte 0 of EEPROM on AVR).
CANnuccia starts on chip reset, reads this id, and sets CAN filters accordingly to listen for commands for the target device; see [docs/CANnuccia.xlsx](docs/CANnuccia.xlsx) for more information on the protocol.  
If no CANnuccia command is received within a timeout (or when a "programming done" command is received), CANnuccia terminates and jumps to the user program.

## Prerequisites
- [CMake](https://cmake.org/) 3.14+

- For STM32: [GNU compiler toolchain for ARM (`arm-none-eabi`)](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) and a standard-compliant `<stdint.h>` header (available for instance from [newlib](https://sourceware.org/newlib/))
- For AVR: [GNU compiler toolchain for AVR](https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers) and the [avr-libc standard library](https://www.nongnu.org/avr-libc/)

## Compiling
Create a build directory and [generate build files via CMake](https://cmake.org/runningcmake/), then compile the project.  

You have to use one of the provided toolchain files (`-DCMAKE_TOOLCHAIN_FILE=`):
- For STM32: `src/stm32/STM32Toolchain.cmake`
- For AVR: `src/avr/AVRToolchain.cmake`

Each toolchain file exposes target-specific configuration options to CMake.

## Goals
- Simplicity and small footprint
    + Written in C99
    + Relies on a minimal amount of dependencies (basically just `<stdint.h>` on STM32)
    + Simple but reliable protocol over CAN bus
- Thin abstraction over target hardware, designed for a minimal amount of work when porting to different platforms
- Complete isolation between the bootloader and the uploaded programs (and no RAM usage after the bootloader terminates)


## License
[Mozilla Public License, Version 2](LICENSE).
