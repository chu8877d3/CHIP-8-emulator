#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <chip8.h>

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_AudioDeviceID audio_device;
} Window;

bool window_init(Window* display, char* title, int width, int height, int scale);
bool window_process_input(bool* keys, SDL_Keycode* key_map);
void window_update_upscale(Window* display, Chip8* chip8);
void window_play_sound(Window* display, uint8_t should_play);
void window_cleanup(Window* display);

#endif