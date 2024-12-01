cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(hello_world
    main.c
    usb_descriptors.c
)

target_include_directories(hello_world PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world pico_stdlib tinyusb_device tinyusb_board pico_multicore)

pico_enable_stdio_usb(hello_world 1)
pico_enable_stdio_uart(hello_world 0)

pico_set_binary_type(hello_world no_flash)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)
