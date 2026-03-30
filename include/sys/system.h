#ifndef SYSTEM_H
#define SYSTEM_H

#include "chip8.h"
#include "config.h"
#include "gui.h"
#include "renderer.h"
#include <SDL.h>

extern const int TIME_FREQUENCY_HZ;

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_PAUSED
} Appstate;

typedef struct AppContext {
    Chip8 chip8;
    Window display;
    GuiState gui;
    Appstate state;
    AppConfig config;
    nk_ctx_t* ctx;
    double fixed_dt; // 时间步长
    double accumlator; // 时间累加器
    double frametime; // 帧时间
    double perf_frequency; // 高精度计时器频率，表示系统底层计时器每秒钟跳动的次数
    Uint64 prev_counter; // 上一次计数器数值
    Uint64 curr_counter; // 最近一次计数器数值
    int cpu_speed; // 模拟器 cpu 运算速度
    int instruction_per_frame; // 单次执行指令数
    bool is_running; // app 是否运行
    bool cpu_speed_change; // cpu speed 是否发生改变
    char cur_running_rom[256]; // 最近一次导入的rom
} AppContext;

void app_init(AppContext* app);
void app_run(AppContext* app);
void app_game_init(AppContext* app);
void app_handle_events(AppContext* app);
void app_game_run(AppContext* app);
void app_exit(AppContext* app);

static inline void app_request_unload_rom(AppContext* app)
{
    app->state = STATE_IDLE;
    app->chip8.running = false;
    chip8_init(&app->chip8);
}
static inline void app_pause_or_resume(AppContext* app)
{
    app->state = (app->state == STATE_PAUSED) ? STATE_RUNNING : STATE_PAUSED;
}


static inline void app_toggle_fullscreen(AppContext* app)
{
    app->gui.is_fullscreen = !app->gui.is_fullscreen;
    if (app->gui.is_fullscreen) {
        SDL_SetWindowFullscreen(app->display.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(app->display.window, 0);
    }
}

#endif