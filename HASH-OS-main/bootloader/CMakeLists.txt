cmake_minimum_required(VERSION 3.10)
project(HASHOS_BOOTLOADER C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Bootloader-specific compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -fno-stack-protector -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

# Architecture-specific flags (assuming x86)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
endif()

add_executable(hashos_bootloader
    bootldr.c
    display.c
    fonts.c
)

target_include_directories(hashos_bootloader PRIVATE .)

# Custom linker script if needed
# set_target_properties(hashos_bootloader PROPERTIES LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld")
