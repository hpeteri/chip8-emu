#ifndef C8_CPU_H
#define C8_CPU_H

#include "c8_inttypes.h"

#define C8_SCREEN_W (64)
#define C8_SCREEN_H (32)
#define C8_PROGRAM_START_ADDR (0x200)

#define C8_ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof(*arr))

/**
 * Structure for CHIP-8 programming language compatible CPU.
 */
struct c8_cpu {

    /* 4096 memory locations 0x1000 */
    uint8_t ram[4096];
    /* Program Counter */
    uint16_t pc;

    /* Program size*/
    uint16_t pc_max;
    
    /* 16 8-bit data registers named V0 to VF */
    uint8_t V[16];
    
    /*Instruction register I, 12 bits wide */
    uint16_t I; 

    /* The stack is only used to store return addresses when
     * subroutines are called.
     */
    uint16_t stack[16];

    /* Stack Pointer */
    uint8_t  sp; 

    /* Two timers, which count down at 60 hertz, until they reach 0. */
    uint16_t delay_timer;
    uint16_t sound_timer;

    /* Hex Keyboard has 16 keys ranging from 0 to F */
    uint8_t keyboard[16];

    uint8_t screen[C8_SCREEN_W * C8_SCREEN_H];
};

#endif /* C8_CPU_H */
