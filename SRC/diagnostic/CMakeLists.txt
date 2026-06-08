cmake_minimum_required(VERSION 3.13)

include(../Pico-GB-20250207/pico_sdk_import.cmake)

project(GB_DIAG C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(../Pico-GB-20250207/ext/FatFs_SPI FatFs_SPI_build)

add_executable(GB_DIAG
    main.c
    ../Pico-GB-20250207/src/mk_ili9225.c
    ../Pico-GB-20250207/ext/i2s/i2s.c
)

pico_generate_pio_header(GB_DIAG
    ${CMAKE_CURRENT_LIST_DIR}/../Pico-GB-20250207/ext/i2s/audio_i2s.pio
)

target_include_directories(GB_DIAG PRIVATE
    ../Pico-GB-20250207/inc
    ../Pico-GB-20250207/ext/i2s
)

target_link_libraries(GB_DIAG
    pico_stdlib
    hardware_spi
    hardware_clocks
    hardware_pio
    hardware_dma
    FatFs_SPI
)

pico_add_extra_outputs(GB_DIAG)
