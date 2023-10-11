# Copyright (c) 2022 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET NuMaker-mbed-SNMP)

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

add_subdirectory(${MBED_PATH})
add_subdirectory(lwip-snmp)

add_executable(${APP_TARGET})

mbed_configure_app_target(${APP_TARGET})

target_include_directories(${APP_TARGET}
    PRIVATE
        app-snmp
        app-snmp/config
        app-snmp/mib
        app-snmp/transport
)

target_sources(${APP_TARGET}
    PRIVATE
        app-snmp/main.cpp
        app-snmp/mib/snmp_gpio_perif_mib.cpp
        app-snmp/transport/snmp_netconn_mbed.cpp
        pre-main/host-stdin/dispatch_host_command.cpp
        pre-main/host-stdin/fetch_host_command.cpp
        pre-main/host-stdin/mem_stats.cpp
        pre-main/host-stdin/pump_host_command.cpp
        pre-main/mbed_main.cpp
        targets/TARGET_NUVOTON/platform_entropy.cpp
)

if("NUVOTON" IN_LIST MBED_TARGET_LABELS)
    target_sources(${APP_TARGET}
        PRIVATE
            "targets/TARGET_NUVOTON/platform_entropy.cpp"
    )
endif()

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-os
        mbed-mbedtls
        mbed-netsocket
        mbed-wifi
        lwip-snmp
)

# Custom target


if("NUMAKER_M467SJ" IN_LIST MBED_TARGET_LABELS)
    add_library(mbed-numaker-m467sj INTERFACE)
    target_include_directories(mbed-numaker-m467sj
        INTERFACE
            TARGET_NUMAKER_M467SJ
    )
    target_link_libraries(mbed-numaker-m467sj INTERFACE mbed-m460)
endif()

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
