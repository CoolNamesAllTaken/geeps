cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(geeps_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(
    -Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
)

add_executable(geeps src/main.cpp)

# Add subdirectories after creating the target so that CMake doesn't get upset.
add_subdirectory(src)
add_subdirectory(inc)

# pull in common dependencies
target_link_libraries(geeps pico_stdlib)

# create map/bin/hex file etc
pico_add_extra_outputs(geeps)
pico_enable_stdio_uart(geeps 1)

target_include_directories(geeps PRIVATE 
    src
    inc
)