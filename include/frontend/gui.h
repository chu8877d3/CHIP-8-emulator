#ifndef GUI_H
#define GUI_H

struct nk_font_atlas;
struct nk_context;
struct AppContext;

#include "chip8.h"
#include "config.h"
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"
#include "window.h"

#define MENU_HEIGHT 30
#define SIDEBAR_WIDTH 250
typedef struct nk_context nk_ctx_t;
typedef struct AppContext AppContext;

typedef enum { POPUP_THEME, POPUP_CPU, POPUP_QUIRKS, POPUP_COUNT } PopupID;

typedef void (*PopupRenderFn)(nk_ctx_t*, AppContext*);

typedef struct {
    const char* title;
    int width, height;
    PopupRenderFn render;
} PopupContrl;

typedef struct GuiState {
    ColorTheme curr_theme_index; // 当前用的主题
    ColorTheme temp_theme_index; // 用于窗口显示的临时主题
    int temp_cpu_speed; // 用于 apply 之前的 speed数值临时存储

    bool popups_active[POPUP_COUNT]; // 弹窗是否显示
    bool popups_changed[POPUP_COUNT]; // 控制各种apply按钮

    bool show_debugger; //  显示degugger侧边栏
    bool show_rom_library; // 显示 rom 目录
    bool show_settings; // 显示设置
    bool show_keymap_help; // 显示 keymap 帮助
    bool is_fullscreen; // 是否是全屏
    char current_rom_path[256]; // 最近导入的存档
} GuiState;

nk_ctx_t* gui_init(AppContext* app);
void gui_render(AppContext* app);

#endif