# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/Users/feder/Git/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(TeensyBridge C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(TeensyBridge
    TeensyBridge.c
    usb_descriptors.c
    )

pico_set_program_name(TeensyBridge "TeensyBridge")
pico_set_program_version(TeensyBridge "0.1")

pico_enable_stdio_uart(TeensyBridge 1)
pico_enable_stdio_usb(TeensyBridge 0)

target_include_directories(TeensyBridge PRIVATE .)

# Add the standard library to the build
target_link_libraries(TeensyBridge
    pico_stdlib
    tinyusb_device
    tinyusb_board
    hardware_uart
    hardware_irq
    )

pico_add_extra_outputs(TeensyBridge)
