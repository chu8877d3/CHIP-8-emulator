#ifndef WINDOW_H
#define WINDOW_H

#include "chip8.h"
#include <SDL.h>

typedef struct Window {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_AudioDeviceID audio_device;
    int window_w;
    int window_h;
} Window;

bool window_init(Window* display, char* title, int width, int height, int scale);
void window_play_sound(Window* display, uint8_t should_play);
void window_cleanup(Window* display);

#endif