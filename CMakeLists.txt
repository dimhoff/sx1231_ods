cmake_minimum_required(VERSION 2.8.8)
project(sx1231_ods C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c11")

# Build Options
set(DEFAULT_DEV_PATH "/dev/spidev0.0" CACHE STRING "Default SPI device path connected to the radio tranciever")
option(WITH_PA1_DEFAULT "Use PA_BOOST(PA1 & PA2) pin by default to transmit(required for RFM69HW)" ON)

configure_file(config.h.in "${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_BINARY_DIR}")

include(GitGetVersion)

add_subdirectory(libsx1231_ods)
add_subdirectory(tools)
