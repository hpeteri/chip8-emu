#ifndef C8_EMU_H
#define C8_EMU_H

struct c8_cpu;

/**
 * @brief Runs emulator
 * @param[in] p_cpu Pointer to CHIP-8 CPU.
 * @return C8_TRUE on success, C8_FALSE otherwise.
 */
int c8_emu_run(struct c8_cpu* p_cpu);

#endif
