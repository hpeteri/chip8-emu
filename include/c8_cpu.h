#ifndef C8_CPU_H
#define C8_CPU_H

#include "c8_inttypes.h"

#define C8_DISP_W (64)
#define C8_DISP_H (32)

/**
 * Structure for CHIP-8 programming language compatible CPU.
 */
struct c8_cpu {

    /* 4096 memory locations 0x1000 */
    uint8_t ram[512];
    /* Program Counter */
    uint16_t pc; 
    
    /* 16 8-bit data registers named V0 to VF */
    uint8_t V[16];
    
    /*Instruction register I, 12 bits wide */
    uint16_t I; 

    /* The stack is only used to store return addresses when
     * subroutines are called. The original RCA 1802 version allocated
     * 48 bytes for up to 12 levels of nesting. Modern implementations
     * usually have more */
    uint16_t stack[48];

    /* Stack Pointer */
    uint8_t  sp; 

    /* Two timers, which count down at 60 hertz, until they reach 0. */
    uint16_t delay_timer;
    uint16_t sound_timer;

    /* Hex Keyboard has 16 keys ranging from 0 to F */
    uint8_t keyboard[16];

    uint8_t display[C8_DISP_W * C8_DISP_H];
};

#endif /* C8_CPU_H */
