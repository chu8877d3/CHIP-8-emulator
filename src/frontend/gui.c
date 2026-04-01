/* 1. 基础配置宏 */
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

/* 2. 关键：触发 SDL 渲染器后端实现代码的宏 */
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION

#include "gui.h"
#include "system.h"
#include "tinyfiledialogs.h"
#include <dirent.h>
#include <string.h>

nk_ctx_t* gui_init(AppContext* app)
{
    Window* display = &app->display;
    GuiState* gui = &app->gui;
    SDL_Window* window = display->window;
    SDL_Renderer* renderer = display->renderer;
    nk_ctx_t* ctx = nk_sdl_init(window, renderer);
    if (!ctx) {
        SDL_Log("Nuklear context: initialization failed!");
        return NULL;
    }

    struct nk_font_atlas* atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font* default_font = nk_font_atlas_add_default(atlas, 13, 0);
    nk_sdl_font_stash_end();

    if (default_font) { nk_style_set_font(ctx, &default_font->handle); }

    gui->is_fullscreen = false;
    gui->show_debugger = true;

    return ctx;
}

static void handle_rom_loading(AppContext* app)
{
    const char* fillter_patherns[] = { "*.ch8", "*.rom", "*.bin" };
    const char* selected_path
        = tinyfd_openFileDialog("请选择 CHIP-8 ROM 文件", "", 3, fillter_patherns, "CHIP-8 ROM 文件 (.ch8, .rom)", 0);

    if (selected_path) {
        if (chip8_load_rom(&app->chip8, selected_path)) {
            strcpy(app->gui.current_rom_path, selected_path);
            app->state = STATE_RUNNING;
            app->is_running = true;
            chip8_restart(&app->chip8);
            app_game_init(app);
        }
    } else {
        tinyfd_messageBox("Error", "load ROM file!", "ok", "error", 1);
    }
}

static void handle_romdir_show(AppContext* app)
{
    (void)app;
    const char* selected_path = tinyfd_selectFolderDialog("请选择 ROM 文件路径", "");
    if (selected_path) { /*pass*/
    }
}

static void gui_render_debugger(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    int win_w = display->window_w;
    int win_h = display->window_h;
    int sidebar_w = SIDEBAR_WIDTH;
    int menu_h = MENU_HEIGHT;

    struct nk_rect bounds = nk_rect(win_w - sidebar_w, menu_h, sidebar_w, win_h - menu_h);
    Chip8* chip8 = &app->chip8;
    if (nk_begin(ctx, "Debugger", bounds, NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        if (nk_tree_push(ctx, NK_TREE_TAB, "Core Register", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "PC: 0x%04x", chip8->pc);
            nk_labelf(ctx, NK_TEXT_LEFT, "I: 0x%04x", chip8->I);
            nk_labelf(ctx, NK_TEXT_LEFT, "SP: 0x%02x", chip8->sp);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Registers V0-VF", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 20, 2);
            uint8_t* V = chip8->V;
            for (int i = 0; i < 16; i++) {
                nk_labelf(ctx, NK_TEXT_LEFT, "V%X: %02x", i, V[i]);
            }
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Timers", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Delay: %d", chip8->delay_timer);
            nk_labelf(ctx, NK_TEXT_LEFT, "Sound: %d", chip8->sound_timer);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Current Instruction", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 25, 1);
            uint16_t opcode = (chip8->memory[chip8->pc] << 8) | (chip8->memory[chip8->pc + 1]);
            nk_labelf(ctx, NK_TEXT_LEFT, "[0x%04x] -> 0x%04x", chip8->pc, opcode);
            nk_tree_pop(ctx);
        }
        nk_end(ctx);
    }
}
static inline void top_menubor_render(nk_ctx_t* ctx, AppContext* app)
{
    Appstate app_state = app->state;
    int win_w = app->display.window_w;

    if (nk_begin(ctx, "MenuBar", nk_rect(0, 0, win_w, MENU_HEIGHT), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {

        nk_menubar_begin(ctx);
        nk_layout_row_static(ctx, MENU_HEIGHT, 70, 5);

        if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 130))) { /*  file */
            nk_layout_row_dynamic(ctx, 25, 1);

            if (nk_menu_item_label(ctx, "Open ROM", NK_TEXT_LEFT)) { // open rom
                handle_rom_loading(app);
            }

            if (nk_menu_item_label(ctx, "Load Directory", NK_TEXT_LEFT)) { // load dir
                handle_romdir_show(app);
            }

            if (app_state == STATE_IDLE) nk_widget_disable_begin(ctx);
            if (nk_menu_item_label(ctx, "Close ROM", NK_TEXT_LEFT)) { // close
                app_request_unload_rom(app);
            }
            if (app_state == STATE_IDLE) nk_widget_disable_end(ctx);

            if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT)) { // exit
                app->is_running = false;
            }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "Emulate", NK_TEXT_LEFT, nk_vec2(120, 160))) { /* emulato r*/
            nk_layout_row_dynamic(ctx, 25, 1);

            if (app_state == STATE_IDLE) nk_widget_disable_begin(ctx);
            if (nk_menu_item_label(ctx, app->state == STATE_PAUSED ? "Resume" : "Pause",
                                   NK_TEXT_LEFT)) { // pause / continue
                app_pause_or_resume(app);
            }

            if (nk_menu_item_label(ctx, "Restart", NK_TEXT_LEFT)) { // restart
                chip8_restart(&app->chip8);
            }
            if (app_state != STATE_IDLE) nk_widget_disable_end(ctx);
            if (nk_menu_item_label(ctx, "CPU Speed (HZ)", NK_TEXT_LEFT)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_property_int(ctx, "CPu Speed (HZ)", 100, &app->cpu_speed, 10000, 10, 100);

                nk_layout_row_dynamic(ctx, 20, 1);
                nk_slider_int(ctx, 100, &app->cpu_speed, 10000, 10);
            }
            if (nk_menu_item_label(ctx, "Quirks", NK_TEXT_LEFT)) { }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "Video", NK_TEXT_LEFT, nk_vec2(120, 130))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_menu_item_label(ctx, "Toggle Fullscreen", NK_TEXT_LEFT)) { app_toggle_fullscreen(app); }

            if (nk_menu_item_label(ctx, "Size", NK_TEXT_LEFT)) { }
            if (nk_menu_item_label(ctx, "Color Theme", NK_TEXT_LEFT)) { }
            if (nk_menu_item_label(ctx, "Aspect", NK_TEXT_LEFT)) { }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "View", NK_TEXT_LEFT, nk_vec2(120, 100))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_checkbox_label(ctx, "Show Debugger", (int*)&app->gui.show_debugger);

            if (nk_menu_item_label(ctx, "Memory Map", NK_TEXT_LEFT)) { }
            if (nk_menu_item_label(ctx, "Log", NK_TEXT_LEFT)) { }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(120, 70))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_menu_item_label(ctx, "Kepmap", NK_TEXT_LEFT)) { }

            if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT)) { }

            nk_menu_end(ctx);
        }
        nk_menubar_end(ctx);
    }
    nk_end(ctx);
}

void gui_render(AppContext* app)
{
    nk_ctx_t* ctx = app->ctx;

    top_menubor_render(ctx, app);
    if (app->gui.show_debugger) { gui_render_debugger(ctx, app); }
}