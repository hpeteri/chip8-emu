cmake_minimum_required(VERSION 3.20)

project(
  chip8-emu
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(chip8-emu)

target_sources(
  chip8-emu
  PRIVATE
  src/main.c
)

target_include_directories(
  chip8-emu
  PRIVATE
  include
)
