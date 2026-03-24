#include "window.h"
#include <math.h>

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
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return false;
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool state = (event.type == SDL_KEYDOWN) ? 1 : 0;
            SDL_Keycode pressed_key = event.key.keysym.sym;
            for (int i = 0; i < 16; i++) {
                if (pressed_key == key_map[i]) {
                    keys[i] = state;
                    break;
                }
            }
        }
    }
    return true;
}

void window_update_upscale(Window* display, const uint32_t* video_buffer)
{
    SDL_UpdateTexture(display->texture, NULL, video_buffer, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(display->renderer);
    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}

void window_play_sound(Window* display, uint8_t should_play)
{
    (void)display;
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
