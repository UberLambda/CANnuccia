# CANnuccia/CMakeLists.txt
#
# Copyright (c) 2019, Paolo Jovon <paolo.jovon@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.14)
project(CANnuccia C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)

add_subdirectory(src/)
