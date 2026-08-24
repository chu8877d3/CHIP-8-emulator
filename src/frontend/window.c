#include "window.h"
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_hints.h>
#include <SDL_log.h>
#ifdef _WIN32
/* SDL_syswm.h 在 Linux 下会拉 X11/Xlib.h，其 Window 类型与本项目 window.h 的 Window 冲突，仅 Windows 需要 */
#include <SDL_syswm.h>
#include <dwmapi.h>
#endif

/* Windows 深色标题栏：与深色 UI 契合（Win10 20H1+ / Win11 用属性 20，1809/1903 用 19），失败则保持系统默认 */
static void window_set_dark_titlebar(SDL_Window* win)
{
#ifdef _WIN32
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(win, &info)) return;
    BOOL dark = TRUE;
    const DWORD attr_20h1 = 20; // DWMWA_USE_IMMERSIVE_DARK_MODE
    const DWORD attr_1809 = 19; // DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
    if (DwmSetWindowAttribute(info.info.win.window, attr_20h1, &dark, sizeof(dark)) != S_OK) {
        DwmSetWindowAttribute(info.info.win.window, attr_1809, &dark, sizeof(dark));
    }
#else
    (void)win;
#endif
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
    // DPI 感知（permonitorv2 + ALLOW_HIGHDPI）：SDL 事件/鼠标坐标/渲染输出全部按物理像素，
    // Nuklear 按物理像素布局后 1:1 上屏，不再被 DWM 拉伸变糊（配合 gui.h 的 gui_ui_scale 等比缩放布局）
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "0"); // 请求尺寸按物理像素解释，不做二次缩放
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("Window %s Init failed %s", title, SDL_GetError());
        return false;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // 按显示器 DPI 放大窗口尺寸，保持与 100% 缩放下相同的视觉大小（内容 1:1 渲染不再被拉伸）
    float dpi_scale = 1.0f;
    float hdpi = 0.0f;
    if (SDL_GetDisplayDPI(0, NULL, &hdpi, NULL) == 0 && hdpi > 0.0f) {
        dpi_scale = hdpi / 96.0f;
    }
    int win_w = (int)(width * scale * dpi_scale + 0.5f);
    int win_h = (int)(height * scale * dpi_scale + 0.5f);
    display->window_h = win_h;
    display->window_w = win_w;
    display->dpi_scale = dpi_scale;

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
    display->window
        = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_w, win_h,
                           SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!display->window) {
        SDL_Log("Window %s create failed %s", title, SDL_GetError());
        return false;
    }
    window_set_dark_titlebar(display->window); // 标题栏改用深色，不再突兀的白色
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
    SDL_SetTextureScaleMode(display->texture, SDL_ScaleModeNearest); // 游戏像素放大保持锐利（不打模糊）
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
