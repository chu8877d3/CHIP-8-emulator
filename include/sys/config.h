#ifndef CONFIG_H
#define CONFIG_H

#include "tools.h"
#include <SDL.h>
#include <cJSON.h>
#include <chip8.h>
/*
    配置文件使用json格式
    配置项包括
    cpu_speed: cpu运行速度
    recently_opened_file: 最近打开文件
    color_theme: 预设主题唯一id
    color_theme-current_background_color: 背景色
    color_theme-current_foreground_color: 前景色
    <自定义主题格式>
    custom_theme数组{
        customkey： {        background_color：
        foreground_color:}

    }
    key_map: 键盘映射
    hotkeys: {

    }
*/
typedef enum ColorTheme {
    THEME_CLASSIC,
    THEME_PHOSPHORGREEN,
    THEME_GAMEBOY,
    THEME_MATRIX,
    THEME_AMBER,
    THEME_VAMPIRE,
    THEME_OCEANIC,
    THEME_COUNT,
    THEME_CUSTOM = 100
} ColorTheme;

typedef struct {
    uint8_t r, g, b, a;
} ColorRGBA;

typedef struct ThemePreset {
    char name[32];
    ColorRGBA bg; // 背景色 (Pixel 0)
    ColorRGBA fg; // 前景色 (Pixel 1)
} ThemeConfig;
static const ThemeConfig THEME_TABLE[THEME_COUNT] = {
    [THEME_CLASSIC] = {
        .name = "Classic Mono",
        .bg = {0, 0, 0, 255},       // 纯黑
        .fg = {255, 255, 255, 255}  // 纯白
    },
    [THEME_PHOSPHORGREEN] = {
        .name = "Phosphor Green",
        .bg = {20, 35, 20, 255},    // 深绿底
        .fg = {51, 255, 51, 255}    // 荧光绿
    },
    [THEME_GAMEBOY] = {
        .name = "GameBoy DMG",
        .bg = {155, 188, 15, 255},  // 经典四阶灰度浅绿
        .fg = {15, 56, 15, 255}     // 经典墨绿
    },
    [THEME_MATRIX] = {
        .name = "Digital Matrix",
        .bg = {0, 0, 0, 255},       // 纯黑
        .fg = {0, 255, 70, 255}     // 矩阵绿
    },
    [THEME_AMBER] = {
        .name = "Amber Monitor",
        .bg = {25, 15, 0, 255},     // 深褐底
        .fg = {255, 176, 0, 255}    // 琥珀金
    },
    [THEME_VAMPIRE] = {
        .name = "Vampire Red",
        .bg = {30, 5, 5, 255},      // 暗红底
        .fg = {255, 30, 30, 255}    // 鲜血红
    },
    [THEME_OCEANIC] = {
        .name = "Oceanic Blue",
        .bg = {10, 25, 40, 255},    // 深海蓝
        .fg = {0, 180, 255, 255}    // 浅天蓝
    }
};
typedef struct AppConfig {
    // 热键设置
    SDL_Keycode key_pause;
    SDL_Keycode key_restart;
    SDL_Keycode key_fullscreen;
    SDL_Keycode key_minimize;
    SDL_Keycode key_debug;

    // 模拟器设置
    bool keymap_is_default;
    SDL_Keycode custom_key_map[16];
    int cpu_frequency_hz;
    ColorTheme theme_type;
    ThemeConfig custom_theme_table[100];
    int custom_theme_count;
    uint32_t color_fg;
    uint32_t color_bg;
    QuirkProfile quirk_mode;

    char current_rom_path[256];
} AppConfig;
#define CONFIG_DEFAULT                                                                                                 \
    { .key_pause = SDLK_F4,                                                                                            \
      .key_restart = SDLK_F5,                                                                                          \
      .key_fullscreen = SDLK_F11,                                                                                      \
      .key_minimize = SDLK_m,                                                                                          \
      .key_debug = SDLK_F3,                                                                                            \
      .theme_type = THEME_CLASSIC,                                                                                     \
      .color_fg = 0xffffffff,                                                                                          \
      .custom_theme_count = 0,                                                                                         \
      .color_bg = 0x000000ff,                                                                                          \
      .quirk_mode = QUIRK_PROFILE_SCHIP_LEGACY,                                                                        \
      .cpu_frequency_hz = 500,                                                                                         \
      .keymap_is_default = true }

AppConfig config_init();
void config_save(AppConfig* config);

static inline void config_apply_theme(AppConfig* config, ColorTheme theme_idx)
{
    const ThemeConfig* t;
    if (theme_idx < THEME_COUNT) {
        t = &THEME_TABLE[theme_idx];
    } else {
        t = &config->custom_theme_table[theme_idx - THEME_COUNT];
    }
    const ColorRGBA* bg = &t->bg;
    const ColorRGBA* fg = &t->fg;
    config->color_bg = color_to_u32(bg->r, bg->g, bg->b, bg->a);
    config->color_fg = color_to_u32(fg->r, fg->g, fg->b, fg->a);
}
#endif