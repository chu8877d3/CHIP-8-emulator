#ifndef GUI_H
#define GUI_H

struct nk_font_atlas;
struct nk_context;
struct AppContext;

#include "chip8.h"
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"
#include "window.h"

#define MENU_HEIGHT 30
#define SIDEBAR_WIDTH 250
typedef struct nk_context nk_ctx_t;
typedef struct AppContext AppContext;

typedef struct GuiState {
    char current_rom_path[256];
    bool show_debugger;
    bool show_rom_library;
    bool show_settings;
    bool show_keymap_help;
    bool is_fullscreen;
} GuiState;

nk_ctx_t* gui_init(AppContext* app);
void gui_render(AppContext* app);

#endif