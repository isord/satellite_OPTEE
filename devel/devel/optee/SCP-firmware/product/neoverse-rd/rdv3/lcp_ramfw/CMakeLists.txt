#
# Arm SCP/MCP Software
# Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rdv3-lcp-bl2)

target_include_directories(
    rdv3-lcp-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/include")

# cmake-lint: disable=E1122

target_sources(
    rdv3-lcp-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_armv8m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu3.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs_handler.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdv3-lcp-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rdv3-lcp-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
