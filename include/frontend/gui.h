#ifndef GUI_H
#define GUI_H

struct nk_font_atlas;
struct nk_context;
struct AppContext;

#include "config.h"
#include "window.h"
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

/* DPI 感知：物理像素下布局等比缩放，保持 100% 缩放的观感（字体按最终像素光栅化，不再被拉伸变糊） */
static inline int gui_ui_scale(Window* w, int v)
{
    return (int)(v * w->dpi_scale + 0.5f);
}
#define MENU_HEIGHT(w) gui_ui_scale((w), 30)
#define SIDEBAR_WIDTH(w) gui_ui_scale((w), 250)
typedef struct nk_context nk_ctx_t;
typedef struct AppContext AppContext;

typedef enum {
    POPUP_THEME,
    POPUP_CPU,
    POPUP_QUIRKS,
    POPUP_ROMLIB,
    POPUP_KEYMAP,
    POPUP_ABOUT,
    POPUP_MEMORY,
    POPUP_COUNT
} PopupID;

typedef void (*PopupRenderFn)(nk_ctx_t*, AppContext*);

typedef struct {
    const char* title;
    int width, height;
    PopupRenderFn render;
} PopupContrl;

typedef struct GuiState {
    struct nk_color nk_bg; // 背景色调色盘
    struct nk_color nk_fg; // 前景色调色盘
    struct nk_color temp_nk_bg; //
    struct nk_color temp_nk_fg; //
    char hex_bg[8]; //
    char hex_fg[8]; //

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

    // ROM 目录浏览器状态
    char rom_dir[256]; // 已选 ROM 目录
    char rom_files[64][256]; // 目录下匹配的 ROM 文件（完整路径）
    int rom_count; // ROM 文件数量
} GuiState;

nk_ctx_t* gui_init(AppContext* app);
void gui_render(AppContext* app);

#endif