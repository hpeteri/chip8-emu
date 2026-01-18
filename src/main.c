#define _POSIX_C_SOURCE 199309L
#define C8_SLEEP_MS(ms)                                                 \
    do {                                                                \
        struct timespec ts = { (ms) / 1000, ((ms) % 1000) * 1000000L }; \
        nanosleep(&ts, NULL);                                           \
    } while (0)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include "c8_cpu.h"

#define INSTRUCTIONS_PER_FRAME 10
#define TERM_ALT_SCREEN_ON  "\033[?1049h"
#define TERM_ALT_SCREEN_OFF "\033[?1049l"
#define TERM_CURSOR_HIDE    "\033[?25l"
#define TERM_CURSOR_SHOW    "\033[?25h"
#define TERM_CURSOR_HOME    "\033[H"
#define TERM_RESET          "\033[0m"

static struct termios orig_termios;

static void terminal_backup_and_setup(
    void);

static void terminal_restore(
    void);

/**
 * @brief atexit cleanup function to restore terminal cursor and color.
 */
static void cleanup(void);

static void handle_interrupt(
    int sig);

static void handle_input(
    struct c8_cpu* p_cpu);

static void print_screen(
    struct c8_cpu* p_cpu);

/* --- Main Function --- */

int main(
    int argc,
    const char* argv[])
{
    int result = C8_TRUE;
    int i;
    struct c8_cpu cpu;

    terminal_backup_and_setup();
    signal(SIGINT,  handle_interrupt); 
    signal(SIGABRT, handle_interrupt); 
    signal(SIGTERM, handle_interrupt);
    atexit(cleanup);
        
    c8_init(&cpu);
    
    if (argc == 2)
    {
        result = c8_load_rom_from_file(argv[1], &cpu);
    }
    else
    {
        printf("usage '%spath/to/rom' \n", argv[0]);
    }

    /* Hide cursor */
    printf("\033[?25l");
    
    /* Run Program */
    while (C8_TRUE == result)
    {
        handle_input(&cpu);
        
        for (i = 0; i < INSTRUCTIONS_PER_FRAME; i++)
        {
            result = c8_step(&cpu);

            if (C8_FALSE == result)
            {
                break;
            }
        }

        if (cpu.screen_is_dirty)
        {
            print_screen(&cpu);
            cpu.screen_is_dirty = 0;
        }
        
        c8_decrement_timers(&cpu);

        if (cpu.sound_timer > 0)
        {
             /* \a is the escape sequence for the system alert/bell */
            printf("\a");
            fflush(stdout); 
        }
        
        /* Sleep to approximately 16.6 ms to maintain 60 FPS */
        C8_SLEEP_MS(16);
    }

    /* atexit called for cleanup */
    return 0;
}

/* --- Local Function Definitions --- */

static void terminal_backup_and_setup(void)
{
    /* Save current state for restore */
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    {
        perror("tcgetattr");
        exit(1);
    }

    struct termios raw = orig_termios;
    
    raw.c_lflag &= ~(ECHO | ICANON);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        perror("tcsetattr");
        exit(1);
    }

    printf(TERM_ALT_SCREEN_ON TERM_CURSOR_HIDE TERM_CURSOR_HOME);
    fflush(stdout);
}

static void terminal_restore(void)
{
    printf(TERM_RESET TERM_CURSOR_SHOW TERM_ALT_SCREEN_OFF);
    fflush(stdout);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/**
 * @brief atexit cleanup function to restore terminal cursor and color.
 */
static void cleanup(void)
{
    terminal_restore();
}

static void handle_interrupt(int sig)
{
    terminal_restore();
    
    if (sig == SIGINT)
    {
        fprintf(stderr, "\nExiting CHIP-8...\n");
        exit(0);
    }

    signal(sig, SIG_DFL);
    raise(sig);
}

static void handle_input(struct c8_cpu* p_cpu)
{
    unsigned char c;

    /* Since the terminal doesnt offer key up event, we keep the key pressed for n frames and release them.
     */
    
    static uint8_t key_timer[16];

    for (int i = 0; i < 16; i++) {
        if (key_timer[i] > 0)
        {
            key_timer[i]--;
        }

        p_cpu->keyboard[i] = (key_timer[i] > 0);
    }

    while (read(STDIN_FILENO, &c, 1) == 1) {
        int key = -1;
        switch (c) {
        case '1': key = 0x1; break; case '2': key = 0x2; break;
        case '3': key = 0x3; break; case '4': key = 0xC; break;
        case 'q': key = 0x4; break; case 'w': key = 0x5; break;
        case 'e': key = 0x6; break; case 'r': key = 0xD; break;
        case 'a': key = 0x7; break; case 's': key = 0x8; break;
        case 'd': key = 0x9; break; case 'f': key = 0xE; break;
        case 'z': key = 0xA; break; case 'x': key = 0x0; break;
        case 'c': key = 0xB; break; case 'v': key = 0xF; break;
        }
        
        if (key != -1)
        {
            key_timer[key] = 12; /* key stays pressed for 12 frames*/
            p_cpu->keyboard[key] = 1;
        }
    }
}

static void print_screen(struct c8_cpu* p_cpu)
{
    /* Move cursor to top-left instead of clearing to avoid flicker */
    printf(TERM_CURSOR_HOME);

    int x, y;
    
    for (y = 0; y < C8_SCREEN_H; y++)
    {
        for (x = 0; x < C8_SCREEN_W; x++)
        {
            if (p_cpu->screen[y * C8_SCREEN_W + x])
            {
                printf("\033[92m██");
            }
            else
            {
                printf("\033[90m░░");
            }
        }

        /* \r is required because raw mode disables automatic carriage return */
        printf("\033[0m\r\n");
    }
    fflush(stdout);
}
