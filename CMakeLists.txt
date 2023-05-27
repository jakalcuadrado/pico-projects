cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME)
project(rushBoard)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(
    ${PROJECT_NAME}
    #blink_led.c
    main.c
    hw_config.c
)

# Tell CMake where to find other source code
add_subdirectory(
    lib/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI 
    #lib/SIM8XX 
    build)
add_subdirectory(lib/ds18b20)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    FatFs_SPI
    pico_one_wire
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)