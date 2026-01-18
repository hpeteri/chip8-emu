cmake_minimum_required(VERSION 3.20)

project(
  chip8-emu
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Terminal version
add_executable(chip8-term)

target_sources(
  chip8-term
  PRIVATE
  src/main.c
  src/c8_cpu.c
)

target_include_directories(
  chip8-term
  PRIVATE
  include
)

# SDL version

find_package(SDL2)

if(SDL2_FOUND)
  add_executable(chip8-sdl EXCLUDE_FROM_ALL)

  target_sources(
    chip8-sdl
    PRIVATE
    src/main_sdl.c
    src/c8_cpu.c
  )

  target_include_directories(
    chip8-sdl
    PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
  )

  target_link_libraries(
    chip8-sdl
    PRIVATE
    ${SDL2_LIBRARIES}
    m
  )
  message(STATUS "SDL2 found: chip8-sdl target is available.")
else()
  message(WARNING "SDL2 not found: chip8-sdl target will not be built.")
endif()
