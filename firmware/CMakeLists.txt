cmake_minimum_required(VERSION 3.13)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Test")

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

    # Source files are added with target_sources in subdirectories
    add_executable(geeps "")

    add_subdirectory(src)
    # add_subdirectory(../modules/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
    

    # In case there are files directly in src and inc
    add_subdirectory(../modules/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)
    target_include_directories(geeps PRIVATE 
        ../modules/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/include
        src
        inc
    )

    # Firmware: Create map/bin/hex file etc
    pico_add_extra_outputs(geeps)
    pico_enable_stdio_usb(geeps 1) # use USB for standard printing
    pico_enable_stdio_uart(geeps 0) # disable STDIO UART

    # Firmware: Pull in Pico library
    target_link_libraries(geeps 
        pico_stdlib
        # FatFs_SPI
        pico_float # for math functions
        pico_multicore
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_gpio
        hardware_irq
        hardware_clocks
        hardware_pwm # for servo
        hardware_adc # for battery voltage
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
    )

    pico_set_linker_script(geeps ${CMAKE_CURRENT_SOURCE_DIR}/linker_scripts/core0_bigstack.ld)

else()
    # Flag indicating that build is not for the embedded target. Used to toggle target_source calls.
    set(ON_HOST 1)

    project(geeps_project C CXX ASM)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)

    add_compile_options(
        -Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
    )

    # Source files are added with target_sources in subdirectories
    add_executable(geeps_test "")

    # Test: Pull in google test library
    add_library(libgtest SHARED IMPORTED)
    set_target_properties(libgtest PROPERTIES IMPORTED_LOCATION /geeps/modules/googletest/build/lib/libgtest.so)
    target_link_libraries(geeps_test PRIVATE libgtest)

    add_subdirectory(test)

endif()
