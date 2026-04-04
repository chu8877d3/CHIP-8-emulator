#include "renderer.h"
#include "system.h"
#include "tools.h"

void render_game(AppContext* app)
{
    Chip8* chip8 = &app->chip8;
    Window* display = &app->display;

    int game_w = chip8->width;
    int game_h = chip8->height;
    const size_t pixel_count = game_w * game_h;

    bool* state = chip8->state;
    uint32_t* video = chip8->video;

    uint32_t color_fg = app->config.color_fg;
    uint32_t color_bg = app->config.color_bg;

    for (size_t i = 0; i < pixel_count; i++) {
        video[i] = state[i] ? color_fg : color_bg;
    }

    SDL_Rect game_rect = { 0, 0, game_w, game_h };
    if (chip8->draw_flag) {
        SDL_UpdateTexture(display->texture, &game_rect, video, game_w * sizeof(uint32_t));
        chip8->draw_flag = false;
    }
    int menu_h = MENU_HEIGHT;
    int win_h = display->window_h;
    int win_w = display->window_w;
    int sidebar_w = app->gui.show_debugger ? SIDEBAR_WIDTH : 0;

    int canvas_w = win_w - sidebar_w;
    int canvas_h = win_h - menu_h;

    SDL_Rect dst_rect;
    if ((float)canvas_w / (float)canvas_h > 2.0f) {
        dst_rect.h = canvas_h;
        dst_rect.w = canvas_h * 2;
    } else {
        dst_rect.w = canvas_w;
        dst_rect.h = canvas_w / 2;
    }
    dst_rect.x = (canvas_w - dst_rect.w) / 2;
    dst_rect.y = menu_h + (canvas_h - dst_rect.h) / 2;

    SDL_RenderCopy(display->renderer, display->texture, &game_rect, &dst_rect);
}

void render_window(AppContext* app)
{
    uint32_t c = app->config.color_bg;
    SDL_SetRenderDrawColor(app->display.renderer, color_get_r(c), color_get_g(c), color_get_b(c), color_get_a(c));
    SDL_RenderClear(app->display.renderer);
    if (app->state == STATE_RUNNING || app->state == STATE_PAUSED) render_game(app);
    SDL_RenderSetLogicalSize(app->display.renderer, 0, 0);
    gui_render(app);
    nk_sdl_render(NK_ANTI_ALIASING_ON);
    SDL_RenderPresent(app->display.renderer);
}
