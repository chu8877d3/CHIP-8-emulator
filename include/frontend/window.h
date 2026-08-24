#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>

typedef struct Window {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_AudioDeviceID audio_device;
    int window_w;   // 客户区大小（物理像素，DPI 感知后与渲染输出 1:1）
    int window_h;
    float dpi_scale; // 显示器 DPI / 96，用于布局等比缩放
} Window;

bool window_init(Window* display, char* title, int width, int height, int scale);
void window_play_sound(Window* display, uint8_t should_play);
void window_cleanup(Window* display);
#endif