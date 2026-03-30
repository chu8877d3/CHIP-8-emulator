#ifndef CONGIG_H
#define CONGIG_H

#include <SDL.h>
#include <chip8.h>

#define MAKE_RGBA(r, g, b, a) (((r) << 24) | ((g) << 16) | ((b) << 8) | a)
#define COLOR_BACKGROUND MAKE_RGBA(0, 5, 0, 255)
#define COLOR_PIXEL MAKE_RGBA(0, 255, 65, 255)

typedef struct AppConfig {
    // 热键设置
    SDL_Keycode key_pause;
    SDL_Keycode key_restart;
    SDL_Keycode key_fullscreen;
    SDL_Keycode key_minimize;
    SDL_Keycode key_debug;

    // 模拟器设置
    bool keymap_is_default;
    int cpu_frequency_hz;
    uint32_t color_pixel;
    uint32_t color_bg;
    QuirkProfile quirk_mode;

    char current_rom_path[256];
} AppConfig;

#define CONFIG_DEFAULT {                      \
    .key_pause = SDLK_F4,                     \
    .key_restart = SDLK_F5,                   \
    .key_fullscreen = SDLK_F11,               \
    .key_minimize = SDLK_m,                   \
    .key_debug = SDLK_F3,                     \
    .color_pixel = COLOR_PIXEL,               \
    .color_bg = COLOR_BACKGROUND,             \
    .quirk_mode = QUIRK_PROFILE_SCHIP_LEGACY, \
    .cpu_frequency_hz = 500,                  \
    .keymap_is_default = true                 \
}

AppConfig config_init();
#endif