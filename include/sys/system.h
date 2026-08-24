#ifndef SYSTEM_H
#define SYSTEM_H

#include "chip8.h"
#include "config.h"
#include "gui.h"
#include "renderer.h"
#include "window.h"
#include <SDL.h>
#include <stdio.h>

extern const int TIME_FREQUENCY_HZ;

typedef enum { STATE_IDLE, STATE_RUNNING, STATE_PAUSED } Appstate;

typedef struct AppContext {
    Chip8 chip8; // chip8 模拟器
    Window display; // 窗口控制
    GuiState gui; // gui 状态
    AppConfig config; // app 配置
    nk_ctx_t* ctx; // gui 上下文
    double fixed_dt; // 时间步长
    double ips_timer; // 记录过了多久
    double accumlator; // 时间累加器
    double ins_per_frame; // 单次执行指令数
    double ins_accumulator; // 指令累加器
    double frametime; // 帧时间
    double perf_frequency; // 高精度计时器频率，表示系统底层计时器每秒钟跳动的次数
    Uint64 prev_counter; // 上一次计数器数值
    Uint64 curr_counter; // 最近一次计数器数值

    Appstate state; // app 状态机
    uint32_t total_ins_count; // 记录一秒跑了多少指令
    uint32_t last_measured_ips; // 最终显示 ips
    int cpu_speed; // 模拟器 cpu 运算速度

    bool is_running; // app 是否运行
    bool cpu_speed_change; // cpu speed 是否发生改变
    bool rom_loaded; // 是否已加载 ROM（重启/恢复运行的依据）
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
    app->rom_loaded = false;
    app->chip8.running = false;
    chip8_init(&app->chip8);
}

/* 统一的 ROM 加载入口：GUI 对话框、ROM 目录浏览器、命令行参数都走这里 */
static inline bool app_load_rom(AppContext* app, const char* path)
{
    if (!chip8_load_rom(&app->chip8, path)) {
        return false;
    }
    snprintf(app->config.current_rom_path, sizeof(app->config.current_rom_path), "%s", path);
    app->rom_loaded = true;
    app->state = STATE_RUNNING;
    chip8_restart(&app->chip8);
    app_game_init(app);
    return true;
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