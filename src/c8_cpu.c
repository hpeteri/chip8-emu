#include "c8_cpu.h"
#include "c8_cpu_local.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

void c8_init(
    struct c8_cpu* p_cpu)
{
    memset(p_cpu, 0x00, sizeof(*p_cpu));

    srand(time(NULL));
}

int c8_load_rom(
    uint32_t       program_size,
    const uint8_t* p_program,
    struct c8_cpu* p_cpu)
{
    if (program_size <= (sizeof(p_cpu->ram) - C8_PROGRAM_START_ADDR))
    {
        memcpy(&p_cpu->ram[C8_PROGRAM_START_ADDR],
               p_program,
               program_size);
    
        printf("Program Size: %"PRIu64" B\n", program_size);
        
        /* Set Program Counter */
        p_cpu->pc = C8_PROGRAM_START_ADDR;
        p_cpu->pc_max = program_size + C8_PROGRAM_START_ADDR;

        return C8_TRUE;
    }
    else
    {
        /* Program does not fit into memory */
        return C8_FALSE;
    }
}

int c8_load_rom_from_file(
    const char*    p_path,
    struct c8_cpu* p_cpu)
{
    int result = C8_TRUE;
    FILE* f = fopen(p_path, "rb");
    size_t file_size;
    size_t program_size;

    printf("Loading rom: [path='%s']\n", p_path);

    if (NULL == f)
    {
        printf("Failed to open file.\n");
        result = C8_FALSE;
    }
    
    /* Check file size */
    if (C8_TRUE == result)
    {
        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        rewind(f);

        if (file_size >
            sizeof(p_cpu->ram) - C8_PROGRAM_START_ADDR)
        {
            printf("Failed to load rom. Program size exceeded [file_size='%"PRIu64"']\n", file_size);
            result = C8_FALSE;
        }
    }

    if (C8_TRUE == result)
    {
        uint8_t ram[4096];
        
        /* Load ROM */
        program_size = fread(ram, 1, file_size, f);

        result = c8_load_rom(program_size,
                             ram,
                             p_cpu);
    }

    /* Close File */
    if (NULL != f)
    {
        fclose(f);
    }

    return result;
}

void c8_load_font(
    struct c8_cpu* p_cpu)
{
    const uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    
    memcpy(p_cpu->ram, fontset, sizeof(fontset));
}

void c8_decrement_timers(
    struct c8_cpu* p_cpu)
{
    if (p_cpu->delay_timer > 0)
    {
        p_cpu->delay_timer--;
    }

    if (p_cpu->sound_timer > 0)
    {
        p_cpu->sound_timer--;
    }
}

int c8_step(
    struct c8_cpu* p_cpu)
{
    int result = C8_TRUE;

    int i;
    int key_pressed;
    uint16_t tmp;
    uint8_t cx, cy;
    
    if (p_cpu->pc >= p_cpu->pc_max)
    {
        printf("unexpected program counter %d / %d \n", p_cpu->pc, p_cpu->pc_max);
        return result;
    }
        
    /* Fetch */
    uint16_t op = (p_cpu->ram[p_cpu->pc] << 8) | p_cpu->ram[p_cpu->pc + 1];
    p_cpu->pc += 2;

    /* Decode */
    const uint8_t type = (op & 0xF000) >> 12;
    const uint8_t x = (op & 0x0F00) >> 8;
    const uint8_t y = (op & 0x00F0) >> 4;
    const uint8_t n = (op & 0x000F);
    const uint8_t nn = (op & 0x00FF);
    const uint16_t nnn  = (op & 0x0FFF);
        
    /* Execute */
    switch (type)
    {
    case 0x00:
        if (op == 0x00E0)
        {
            /* Opcode: 00E0
             * Clears the screen
             */
            memset(p_cpu->screen, 0x00, sizeof(p_cpu->screen));
            p_cpu->screen_is_dirty = 1;
        }
        else if (op == 0x00EE)
        {
            /* Opcode: 00EE
             * Returns from a subroutine
             */
            assert(0 != p_cpu->sp);

            p_cpu->sp--;
            p_cpu->pc = p_cpu->stack[p_cpu->sp];
        }
        else
        {
            /* Opcode: 0NNN
             * Calls machine code routine at address NNN
             */

            /* Treat as NOP */
        }
            
        break;

    case 0x1:
        /* Opcode: 0x1NNN
         * Jumps to adddress NNN
         */
        p_cpu->pc = nnn;
        break;
            
    case 0x2:
        /* Opcode: 0x2NNN
         * Calls subroutine at NNN
         */
        assert(p_cpu->sp < C8_ARRAY_SIZE(p_cpu->stack));

        p_cpu->stack[p_cpu->sp] = p_cpu->pc;
        p_cpu->sp++;
        p_cpu->pc = nnn;

        break;
                        
    case 0x3:
        /* Opcode: 0x3XNN
         * Skips the next instruction if VX equals NN
         * if (VX == NN)
         */
        if (p_cpu->V[x] == nn)
        {
            p_cpu->pc += 2;
        }
        break;

    case 0x4:
        /* Opcode: 0x4XNN
         * Skips the next instruction if VX does not equal NN
         * if (VX != NN)
         */
        if (p_cpu->V[x] != nn)
        {
            p_cpu->pc += 2;
        }
        break;

    case 0x5:
        /* Opcode: 0x5XY0
         * Skips the next instruction if VX equal VY
         * if (VX == VY)
         */
        if (p_cpu->V[x] == p_cpu->V[y])
        {
            p_cpu->pc += 2;
        }
        break;
                        
    case 0x6:
        /* Opcode: 0x6XNN
         * Sets VX to NN
         */
        assert(x < 16);
        p_cpu->V[x] = nn;
        break;

    case 0x7:
        /* Opcode: 0x7XNN
         * Adds NN to VX (carry flag is not changed)
         */
        p_cpu->V[x] += nn;
        break;
            
    case 0x8:
        switch (n)
        {
        case 0x0:
            /* Opcode: 0x8XY0
             * Sets VX to the value of VY
             */
            p_cpu->V[x] = p_cpu->V[y];
            break;

        case 0x1:
            /* Opcode: 0x8XY1
             * Sets VX to VX or VY. (bitwise OR)
             */
            p_cpu->V[x] = p_cpu->V[x] | p_cpu->V[y];
            break;
                
        case 0x2:
            /* Opcode: 0x8XY2
             * Sets VX to VX and VY. (bitwise AND)
             */
            p_cpu->V[x] = p_cpu->V[x] & p_cpu->V[y];
            break;
                
        case 0x3:
            /* Opcode: 0x8XY3
             * Sets VX to VX xor VY. (bitwise XOR)
             */
            p_cpu->V[x] = p_cpu->V[x] ^ p_cpu->V[y];
            break;
                
        case 0x4:
            /* Opcode: 0x8XY4.
             * Adds VY to VX. VF is set to 1 when there is an
             * overflow, and to 0 when there is not.
             */
            tmp = p_cpu->V[x] + p_cpu->V[y];
            p_cpu->V[x] = (uint8_t)tmp;
            p_cpu->V[15] = (tmp > 0xff);

            break;
                
        case 0x5:
            /* Opcode: 0x8XY5
             *
             * VY is subtracted from VX. VF is set to 0 when there
             * is an underflow, and 1 when there is not.
             */
            tmp = p_cpu->V[x] >= p_cpu->V[y];
            p_cpu->V[x] = p_cpu->V[x] - p_cpu->V[y];
            p_cpu->V[15] = tmp;
            break;
                
        case 0x6:
            /* Opcode: 0x8XY6
             *
             * Shifts VX to the right by 1, then stores the least
             * significant bit of VX prior to the shift into VF
             */
            tmp = p_cpu->V[x] & 1;
            p_cpu->V[x] >>= 1;
            p_cpu->V[15] = tmp;
            break;
                
        case 0x7:
            /* Opcode: 0x8XY7
             *
             * Sets VX to VY minus VX. VF is set to 0 when there's
             * an underflow, and 1 when there is not.
             */
            tmp = p_cpu->V[y] >= p_cpu->V[x];
            p_cpu->V[x] = p_cpu->V[y] - p_cpu->V[x];
            p_cpu->V[15] = tmp;
            break;
                
        case 0xE:
            /* Opcode: 0x8XYE
             *
             * Shifts VX to the left by 1, then stores the most
             * significant bit of VX prior to the shift into VF
             */
            tmp = (p_cpu->V[x] >> 7);
            p_cpu->V[x] <<= 1;
            p_cpu->V[15] = tmp;
            break;
                
        default:
            assert(0);
            break;
        }
        break;
            
    case 0x9:
        /* Opcode: 0x9XY0
         * Skips the next instruction if VX does not  equal VY.
         * if (VX != VY)
         */
        if (p_cpu->V[x] != p_cpu->V[y])
        {
            p_cpu->pc += 2;
        }
            
        break;

    case 0xA:
        /* Opcode: 0xANNN
         * Sets I to the address NNN
         */
        p_cpu->I = nnn;
        break;
            
    case 0xB:
        /* Opcode: 0xBNNN
         * Jumps to the address NNN plus V0
         */
        p_cpu->pc = nnn + p_cpu->V[0];
        break;
            
    case 0xC:
        /* Opcode: 0xCXNN
         * Sets VX to the result of a bitwise and operation on a random number and NN
         * (VX = rand() & NN)
         */
        // 2. Perform the bitwise AND with the constant NN
        p_cpu->V[x] = rand() & nn;
        break;

    case 0xD:
        /* Opcode: 0xDXYN
         * Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
         *
         * Each row of 8 pixels is read as bit-coded starting from memory location I.
         * VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn
         */

        c8_draw(p_cpu, x, y, n);
        p_cpu->screen_is_dirty = 1;
                
        break;

    case 0xE:
        if (0x9E == nn)
        {
            /* Opcode: 0xEx9E
             * Skips the next instruction if the key stored in VX is pressed
             */
            if (p_cpu->keyboard[p_cpu->V[x & 0xF] & 0xF])
            {
                p_cpu->pc += 2;
            }
        }
        else if (0xA1 == nn)
        {
            /* Opcode: 0xExA1
             * Skips the next instruction if the key stored in VX is not pressed
             */
            if (!p_cpu->keyboard[p_cpu->V[x & 0xF] & 0xF])
            {
                p_cpu->pc += 2;
            }
        }
        else
        {
            assert(0);
        }
        break;
    case 0xF:
        if (0x07 == nn)
        {
            /* Opcode: 0xFX07
             * Sets VX to the value of the delay timer
             */

            p_cpu->V[x] = p_cpu->delay_timer;
        }
        else if (0x0A == nn)
        {
            /* Opcode: 0xFX0A
             * Wait for a key press, store in VX
             */

            for (i = 0; i < 16; i++)
            {
                if (p_cpu->keyboard[i])
                {
                    key_pressed = i;
                    break;
                }
            }

            if (key_pressed != -1)
            {
                p_cpu->V[x] = (uint8_t)key_pressed;
            }
            else
            {
                /* Wait until key press */
                p_cpu->pc -= 2;
            }
        }
        else if (0x15 == nn)
        {
            /* Opcode: 0xFX15
             * Sets the delay timer to VX
             */
            p_cpu->delay_timer = p_cpu->V[x];
        }
        else if (0x18 == nn)
        {
            /* Opcode: 0xFX18
             * Sets the sound timer to VX
             */
            p_cpu->sound_timer = p_cpu->V[x];
        }
        else if (0x1E == nn)
        {
            /* Opcode: 0xFX1E
             * Adds VX to I. VF is not affected.
             */
            p_cpu->I += p_cpu->V[x];
        }
        else if (0x29 == nn)
        {
            /* Opcode: 0xFX29
             * Sets I to the location of the sprite for the character in VX.
             * Characters 0-F (in hexadecimal) are represented  by a 4x5 font.
             */
            p_cpu->I = p_cpu->V[x] * 5;
        }
        else if (0x33 == nn)
        {
            /* Opcode: 0xFX33
             *
             * Stores the binary-coded decimal representation of
             * VX, with the hundreds digit in memory at location
             * in I, the tens digit at location I+1, and the ones
             * digit at location I+2
             */
            uint8_t value = p_cpu->V[x];
            p_cpu->ram[p_cpu->I]     = value / 100;         
            p_cpu->ram[p_cpu->I + 1] = (value / 10) % 10;   
            p_cpu->ram[p_cpu->I + 2] = value % 10;          
        }
        else if (0x55 == nn)
        {
            /* Opcode: 0xFX55
             *
             * Stores from V0 to VX (including) in memory, starting at address I.
             * I is not modified.
             */
            c8_reg_dump(p_cpu, x);
        }
        else if (0x65 == nn)
        {
            /* Opcode: 0xFX66
             *
             * Fills from V0 to VX (including) with values from memory, starting at address I.
             * I is not modified.
             */
            c8_reg_load(p_cpu, x);
        }
        else
        {
            assert(0);
        }
        break;
    default:
        assert(0);
        break;
    }
    
    return result;
}

/* --- Local Function Definitions --- */

static void c8_draw(
    struct c8_cpu* p_cpu,
    uint8_t x,
    uint8_t y,
    uint8_t n)
{
    uint8_t x_pos = p_cpu->V[x] % C8_SCREEN_W;
    uint8_t y_pos = p_cpu->V[y] % C8_SCREEN_H;
    
    p_cpu->V[0xF] = 0;

    for (int row = 0; row < n; row++) {
        uint8_t sprite_byte = p_cpu->ram[p_cpu->I + row];

        for (int col = 0; col < 8; col++) {
            if ((sprite_byte & (0x80 >> col)) != 0) {
                uint16_t screen_idx = ((y_pos + row) % C8_SCREEN_H) * C8_SCREEN_W + ((x_pos + col) % C8_SCREEN_W);

                if (p_cpu->screen[screen_idx] == 1) {
                    p_cpu->V[0xF] = 1;
                }

                p_cpu->screen[screen_idx] ^= 1;
            }
        }
    }

    p_cpu->screen_is_dirty = 1;
}

static void c8_reg_dump(
    struct c8_cpu* p_cpu,
    uint8_t x)
{
    uint8_t i;
    for (i = 0; i <= x; i++)
    {
        p_cpu->ram[p_cpu->I + i] = p_cpu->V[i];
    }
}
    
static void c8_reg_load(
    struct c8_cpu* p_cpu,
    uint8_t x)
{
    uint8_t i;
    for (i = 0; i <= x; i++)
    {
        p_cpu->V[i] = p_cpu->ram[p_cpu->I + i];
    }
}
