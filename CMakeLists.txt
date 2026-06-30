cmake_minimum_required(VERSION 3.16)
project(iota_gbc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    bus.cpp
    cpu.cpp
    cpu_alu.cpp
    ppu.cpp
)

add_executable(boot_run ${SOURCES})

if(MSVC)
    target_compile_options(boot_run PRIVATE /W4)
else()
    target_compile_options(boot_run PRIVATE -Wall -Wextra)
endif()