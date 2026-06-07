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
)

target_include_directories(GB_DIAG PRIVATE
    ../Pico-GB-20250207/inc
)

target_link_libraries(GB_DIAG
    pico_stdlib
    hardware_spi
    hardware_clocks
    FatFs_SPI
)

pico_add_extra_outputs(GB_DIAG)
