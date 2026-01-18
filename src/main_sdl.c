#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "c8_cpu.h"

#define WINDOW_SCALE 15
#define INSTRUCTIONS_PER_FRAME 10

#define SAMPLE_RATE 44100
#define AMPLITUDE 8000   
#define FREQUENCY 440.0f 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* --- Local Function Declarations --- */

static void handle_input(
    struct c8_cpu* p_cpu, 
    int* p_quit);

static void draw_screen(
    struct c8_cpu* p_cpu, 
    SDL_Texture* p_texture, 
    SDL_Renderer* p_renderer);

static void audio_callback(
    void* userdata,
    uint8_t* stream,
    int len);

/* --- Main Function --- */

int main(
    int argc, 
    char* argv[])
{
    struct c8_cpu cpu;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    int result = C8_TRUE;
    int i;
    int quit = 0;
    SDL_AudioSpec want;

    if (argc != 2)
    {
        printf("usage: ./chip8-sdl path/to/rom\n");
        return 1;
    }

    /* Initialize CPU */
    c8_init(&cpu);
    result = c8_load_rom_from_file(argv[1], &cpu);

    if (C8_FALSE == result)
    {
        return 1;
    }

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    /* Audio */
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audio_callback;
    want.userdata = &cpu;

    if (SDL_OpenAudio(&want, NULL) < 0)
    {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
    }

    /* Window */
    window = SDL_CreateWindow(
        "CHIP-8 Emulator (SDL)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        C8_SCREEN_W * WINDOW_SCALE,
        C8_SCREEN_H * WINDOW_SCALE,
        SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING, 
        C8_SCREEN_W, 
        C8_SCREEN_H);

    /* Main Loop */
    while (C8_TRUE == result && !quit)
    {
        handle_input(&cpu, &quit);

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
            draw_screen(&cpu, texture, renderer);
            cpu.screen_is_dirty = 0;
        }

        c8_decrement_timers(&cpu);

        if (cpu.sound_timer > 0)
        {
            SDL_PauseAudio(0);
        }
        else
        {
            SDL_PauseAudio(1);
        }

        SDL_Delay(16);
    }

    /* Cleanup */
    SDL_CloseAudio();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

/* --- Local Function Definitions --- */

static void audio_callback(void* userdata, uint8_t* stream, int len)
{
    struct c8_cpu* cpu = (struct c8_cpu*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int length = len / 2; 
    static float phase = 0.0f;
    
    /* Calculate how much the phase moves per sample */
    float phase_increment = (2.0f * M_PI * FREQUENCY) / SAMPLE_RATE;

    for (int i = 0; i < length; i++)
    {
        /* Generate Sine Wave: amplitude * sin(phase) */
        buffer[i] = (int16_t)(AMPLITUDE * sinf(phase));
        
        phase += phase_increment;
        
        if (phase >= 2.0f * M_PI)
        {
            phase -= 2.0f * M_PI;
        }
    }
}

static void handle_input(struct c8_cpu* p_cpu, int* p_quit)
{
    SDL_Event e;
    int is_down;
    
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            *p_quit = 1;
        }

        if (e.type == SDL_KEYDOWN ||
            e.type == SDL_KEYUP)
        {
            is_down = (e.type == SDL_KEYDOWN);

            if (is_down &&
                e.key.keysym.sym == SDLK_ESCAPE)
            {
                *p_quit = 1;
                break;
            }

            switch (e.key.keysym.sym)
            {
            case SDLK_1: p_cpu->keyboard[0x1] = is_down; break;
            case SDLK_2: p_cpu->keyboard[0x2] = is_down; break;
            case SDLK_3: p_cpu->keyboard[0x3] = is_down; break;
            case SDLK_4: p_cpu->keyboard[0xC] = is_down; break;

            case SDLK_q: p_cpu->keyboard[0x4] = is_down; break;
            case SDLK_w: p_cpu->keyboard[0x5] = is_down; break;
            case SDLK_e: p_cpu->keyboard[0x6] = is_down; break;
            case SDLK_r: p_cpu->keyboard[0xD] = is_down; break;

            case SDLK_a: p_cpu->keyboard[0x7] = is_down; break;
            case SDLK_s: p_cpu->keyboard[0x8] = is_down; break;
            case SDLK_d: p_cpu->keyboard[0x9] = is_down; break;
            case SDLK_f: p_cpu->keyboard[0xE] = is_down; break;

            case SDLK_z: p_cpu->keyboard[0xA] = is_down; break;
            case SDLK_x: p_cpu->keyboard[0x0] = is_down; break;
            case SDLK_c: p_cpu->keyboard[0xB] = is_down; break;
            case SDLK_v: p_cpu->keyboard[0xF] = is_down; break;

            default: break;
            }
        }
    }
}

static void draw_screen(struct c8_cpu* p_cpu, SDL_Texture* p_texture, SDL_Renderer* p_renderer)
{
    uint32_t pixels[C8_SCREEN_W * C8_SCREEN_H];
    int i;

    for (i = 0; i < C8_SCREEN_W * C8_SCREEN_H; i++)
    {
        pixels[i] = (p_cpu->screen[i]) ? 0x00FF00FF : 0x1A1A1AFF;
    }

    SDL_UpdateTexture(p_texture, NULL, pixels, C8_SCREEN_W * sizeof(uint32_t));
    SDL_RenderClear(p_renderer);
    SDL_RenderCopy(p_renderer, p_texture, NULL, NULL);
    SDL_RenderPresent(p_renderer);
}
