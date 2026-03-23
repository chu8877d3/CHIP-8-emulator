#include "chip8.h"
#include "window.h"

SDL_Keycode keyMap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v
};


const int timer_frequency_hz = 60;
int cpu_frequency_hz = 600;
int main(int argc, char* argv[])
{
    
    Chip8 chip8;
    chip8_init(&chip8);
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <rom_path> [--old|--new]\n", argv[0]);
        return -1;
    }

    if (!chip8_load_rom(&chip8, argv[1])) {
        return -1;
    }
    
    chip8.shift_quirk = true;
    if (argc == 3) {
        if (strcmp("--old", argv[2]) == 0) {
            chip8.shift_quirk = false;
        } else if (strcmp("--new", argv[2]) == 0) {
            chip8.shift_quirk = true;
        } else {
            fprintf(stderr, "Unkown option: %s\n", argv[2]);
            return -1;
        }
    }
    const int instruction_count_with_s = cpu_frequency_hz / timer_frequency_hz;
    Window display;
    if (!window_init(&display, "CHIP8-EMULATOR", SCREEN_WIDTH, SCREEN_HEIGHT, 10)) {
        return 1;
    }
    const double FIXED_DT = 1.0 / (double)timer_frequency_hz;

    Uint64 prevCounter = SDL_GetPerformanceCounter();
    double accmulator = 0.0;
    double freq = (double)SDL_GetPerformanceFrequency();

    bool isRuning = true;
    while (isRuning) {
        Uint64 currCounter = SDL_GetPerformanceCounter();
        double frameTime = (currCounter - prevCounter) / freq;
        prevCounter = currCounter;

        if (frameTime > 0.25)
            frameTime = 0.25;

        accmulator += frameTime;

        isRuning = window_process_input(chip8.keypad, keyMap);
        while (accmulator >= FIXED_DT) {
            if (!isRuning)
                break;
            for (int i = 0; i < instruction_count_with_s; i++) {
                chip8_cycle(&chip8);
            }
            update_timers(&chip8);
            accmulator -= FIXED_DT;
        }
        if (isRuning) {
            window_play_sound(&display, chip8.sound_timer);
            window_update_upscale(&display, chip8.video);
        }
    }
    window_cleanup(&display);
    return 0;
}