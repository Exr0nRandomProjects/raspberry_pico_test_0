cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(my_proj
    main.c
)
# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(my_proj pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(my_proj 1)
pico_enable_stdio_uart(my_proj 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(my_proj)

# add url via pico_set_program_url
#example_auto_set_url(hello_usb)
