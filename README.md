# Chip8-emu

A portable CHIP-8 emulator written in C.
https://en.wikipedia.org/wiki/CHIP-8

# Overview

The emulator follow the standard CHIP-8 specifications:
* 4KB of RAM
* 16 8-bit general-purpose registers (V0-VF)
* 16-bit Program Counter (PC) and Index Register (I)
* A 16-level stack for subroutiner
* 64x32 monochrome display buffer.
* 60Hz delay and sound timers.

# How it Works

The CPU runs a loop that performs three steps: Fetch, Decode, and Execute

# Building

To compile, run `cmake -S . -B <build>` to generate the build files. Compile the project with `cmake --build <build>`

# Usage

Once compiled, run the emulator by passing the path to a CHIP-8 ROM file: `./chip8-emu path/to/rom.ch8`