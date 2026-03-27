#include "window.h"
#include <math.h>

static inline int map_keycode_to_chip8(SDL_Keycode pressed_key)
{
    switch (pressed_key) {
    case SDLK_x: return 0x0;
    case SDLK_1: return 0x1;
    case SDLK_2: return 0x2;
    case SDLK_3: return 0x3;
    case SDLK_q: return 0x4;
    case SDLK_w: return 0x5;
    case SDLK_e: return 0x6;
    case SDLK_a: return 0x7;
    case SDLK_s: return 0x8;
    case SDLK_d: return 0x9;
    case SDLK_z: return 0xA;
    case SDLK_c: return 0xB;
    case SDLK_4: return 0xC;
    case SDLK_r: return 0xD;
    case SDLK_f: return 0xE;
    case SDLK_v: return 0xF;
    default: return -1;
    }
}

static void audio_callback(void* userdata, Uint8* stream, int len)
{
    (void)userdata;
    Sint16* buffer = (Sint16*)stream;
    int length = len / 2;

    static int sample_index = 0;
    int frequency = 440; // 蜂鸣器频率 （440Hz 标准A音)
    int sample_rate = 44100; // 采样率
    int volume = 3000; // 音量

    for (int i = 0; i < length; i++) {
        if ((sample_index++ / (sample_rate / frequency / 2) % 2)) {
            buffer[i] = volume;
        } else {
            buffer[i] = -volume;
        }
    }
}

bool window_init(Window* display, char* title, int width, int height, int scale)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        SDL_Log("Window %s Init failed %s", title, SDL_GetError());
        return false;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audio_callback;

    display->audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!display->audio_device) {
        SDL_Log("Audio init failed%s", SDL_GetError());
        return false;
    }
    display->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * scale, height * scale, SDL_WINDOW_SHOWN);
    if (!display->window) {
        SDL_Log("Window %s create failed %s", title, SDL_GetError());
        return false;
    }
    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
    if (!display->renderer) {
        SDL_DestroyWindow(display->window);
        SDL_Log("Renderer %s create failed %s", title, SDL_GetError());
        return false;
    }
    display->texture = SDL_CreateTexture(display->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!display->texture) {
        SDL_DestroyWindow(display->window);
        SDL_DestroyRenderer(display->renderer);
        SDL_Log("Texture %s create failed %s", title, SDL_GetError());
        return false;
    }
    return true;
}

bool window_process_input(bool* keys, SDL_Keycode* key_map)
{
    (void)key_map;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return false;
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if (event.type == SDL_KEYDOWN && event.key.repeat != 0) {
                continue;
            }

            bool state = (event.type == SDL_KEYDOWN) ? 1 : 0;
            int key_index = map_keycode_to_chip8(event.key.keysym.sym);
            if (key_index >= 0) {
                keys[key_index] = state;
            }
        }
    }
    return true;
}

void window_update_upscale(Window* display, Chip8* chip8)
{
    const size_t pixel_count = chip8->height * chip8->width;
    uint32_t* video = chip8->video;
    const bool* state = chip8->state;
    for (size_t i = 0; i < pixel_count; i++) {
        video[i] = state[i] ? COLOR_PIXEL : COLOR_BACKGROUND;
    }
    SDL_Rect active_rect = { 0, 0, chip8->width, chip8->height };

    SDL_UpdateTexture(display->texture, &active_rect, chip8->video, chip8->width * sizeof(uint32_t));
    SDL_RenderSetLogicalSize(display->renderer, chip8->width, chip8->height);
    SDL_RenderCopy(display->renderer, display->texture, &active_rect, NULL);
    SDL_RenderPresent(display->renderer);
}

void window_play_sound(Window* display, uint8_t should_play)
{
    if (should_play)
        SDL_PauseAudioDevice(display->audio_device, 0);
    else
        SDL_PauseAudioDevice(display->audio_device, 1);
}

void window_cleanup(Window* display)
{
    if (display->audio_device != 0) {
        SDL_CloseAudioDevice(display->audio_device);
    }
    SDL_DestroyWindow(display->window);
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyTexture(display->texture);
    SDL_Quit();
}
