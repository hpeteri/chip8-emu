#include <stdio.h>
#include "c8_cpu.h"
#include "c8_emu.h"

#include <string.h> /* memset */

/* --- Local Function Declarations --- */
static int c8_load_rom(
    const char*    p_path,
    struct c8_cpu* p_cpu);

/* --- Main Function --- */

int main(
    int argc,
    const char* argv[])
{
    int result = C8_TRUE;
    struct c8_cpu cpu;

    /* Load ROM */
    if (argc == 2)
    {
        result = c8_load_rom(argv[1], &cpu);
    }
    else
    {
        printf("usage './chip8-emu path/to/rom' \n");
    }

    /* Run Program */
    if (C8_TRUE == result)
    {
        result = c8_emu_run(&cpu);

        if (C8_FALSE == result)
        {
            printf("Error encountered when running emulator.\n");
        }
    }

    
    return 0;
}

/* --- Local Function Definitions --- */

/**
 * @brief opens rom file from path and loads it to memory
 * @param[in] p_path, path/to/rom. Must not be NULL.
 * @param[out] p_cpu, Pointer to CPU. Must not be NULL.
 * @return C8_TRUE on success, C8_FALSE otherwise
 */
static int c8_load_rom(
    const char* p_path,
    struct c8_cpu* p_cpu)
{
    int result = C8_TRUE;
    FILE* f = fopen(p_path, "rb");
    size_t file_size;
    size_t program_size;

    /* Clear memory */
    memset(p_cpu, 0x00, sizeof(*p_cpu));

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
        /* Load ROM */
        program_size = fread(&p_cpu->ram[C8_PROGRAM_START_ADDR], 1, file_size, f);
        printf("Program Size: %"PRIu64" B\n", program_size);
        
        /* Set Program Counter */
        p_cpu->pc = C8_PROGRAM_START_ADDR;
    }

    /* Close File */
    if (NULL != f)
    {
        fclose(f);
    }

    return result;
}
