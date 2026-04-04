#include "window.h"
#include "tools.h"
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_hints.h>
#include <SDL_log.h>
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
    display->window_h = height * scale;
    display->window_w = width * scale;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
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
    display->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * scale,
                                       height * scale, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!display->window) {
        SDL_Log("Window %s create failed %s", title, SDL_GetError());
        return false;
    }
    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!display->renderer) {
        SDL_DestroyWindow(display->window);
        SDL_Log("Renderer %s create failed %s", title, SDL_GetError());
        return false;
    }
    display->texture
        = SDL_CreateTexture(display->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!display->texture) {
        SDL_DestroyWindow(display->window);
        SDL_DestroyRenderer(display->renderer);
        SDL_Log("Texture %s create failed %s", title, SDL_GetError());
        return false;
    }
    return true;
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
