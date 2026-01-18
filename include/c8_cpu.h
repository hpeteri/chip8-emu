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
 * https://en.wikipedia.org/wiki/CHIP-8
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

    /* flag for then the screen should be updated */
    int screen_is_dirty;
};

/**
 * @brief Initialize CHIP-8 CPU struct.
 * @param[out] p_cpu, Pointer to CHIP-8 CPU struct. Must not be NULL.
 */
void c8_init(
    struct c8_cpu* p_cpu);

/**
 * @brief - Load program into memory, starting from 0x200
 * @param[in] program_size
 * @param[in] p_program, Pointer to program
 * @param[out] p_cpu, Pointer to CHIP-8 CPU struct
 * @return C8_TRUE on success, C8_FALSE otherwise.
 */
int c8_load_rom(
    uint32_t       program_size,
    const uint8_t* p_program,
    struct c8_cpu* p_cpu);

/**
 * @brief opens rom file from path and loads it to memory
 * @param[in] p_path, path/to/rom. Must not be NULL.
 * @param[out] p_cpu, Pointer to CPU. Must not be NULL.
 * @return C8_TRUE on success, C8_FALSE otherwise
 */
int c8_load_rom_from_file(
    const char* p_path,
    struct c8_cpu* p_cpu);

/**
 * @brief - Load font into memory, starting from 0
 * @param[out] p_cpu, Pointer to CHIP-8 CPU struct
 */
void c8_load_font(
    struct c8_cpu* p_cpu);

/**
 * @brief Decrement timers. Should be halled at 60Hz.
 * @param[out] p_cpu, Pointer to CHIP-8 CPU struct
 */
void c8_decrement_timers(
    struct c8_cpu* p_cpu);

/**
 * @brief Steps the CPU for a single instruction
 * @param[in] p_cpu Pointer to CHIP-8 CPU.
 * @return C8_TRUE if CPU is still running, C8_FALSE if error is encountered or on exit.
 */
int c8_step(struct c8_cpu* p_cpu);

#endif /* C8_CPU_H */
