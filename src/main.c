#include "chip8.h"
#include "window.h"

static SDL_Keycode key_map[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

static const int TIME_FREQUENCY_HZ = 60;

int main(int argc, char* argv[])
{
    Chip8 chip8;
    chip8_init(&chip8);
    int cpu_frequency_hz = 600;

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
    const int INSTRUCTION_PER_FRAME = cpu_frequency_hz / TIME_FREQUENCY_HZ; // 单步执行指令数
    Window emulator_window;
    if (!window_init(&emulator_window, "CHIP8-EMULATOR", SCREEN_WIDTH, SCREEN_HEIGHT, 10)) {
        return 1;
    }
    const double FIXED_DT = 1.0 / (double)TIME_FREQUENCY_HZ; // 固定时间步长

    Uint64 prev_counter = SDL_GetPerformanceCounter(); // 上一帧计数器数值
    double accumulator = 0.0; // 时间累加器
    double perf_frequenency = (double)SDL_GetPerformanceFrequency(); // 高精度计时器频率，表示系统底层计时器每秒钟跳动的次数

    bool is_running = true;
    while (is_running) {
        Uint64 curr_counter = SDL_GetPerformanceCounter();
        double frame_time = (curr_counter - prev_counter) / perf_frequenency; // 帧时间
        prev_counter = curr_counter;

        if (frame_time > 0.25)
            frame_time = 0.25; // 防止fram_tiem变得及其大，积压大量逻辑导致单帧率运算卡死

        accumulator += frame_time;

        is_running = window_process_input(chip8.keypad, key_map);
        while (accumulator >= FIXED_DT) {
            if (!is_running)
                break;
            for (int i = 0; i < INSTRUCTION_PER_FRAME; i++) {
                chip8_cycle(&chip8);
            }
            update_timers(&chip8);
            accumulator -= FIXED_DT;
        }
        if (is_running) {
            window_play_sound(&emulator_window, chip8.sound_timer);
            if (chip8.draw_flag) {
                window_update_upscale(&emulator_window, chip8.video);
                chip8.draw_flag = false;
            }
        }
    }
    window_cleanup(&emulator_window);
    return 0;
}