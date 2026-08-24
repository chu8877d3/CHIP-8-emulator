/*  基础配置宏 */
#include <sys/types.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "gui.h"
#include "system.h"
#include "tinyfiledialogs.h"
#include "tools.h"
#include "window.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

/* GUI 深色主题：深板岩底 + 荧光绿点缀（NK_COLOR_* 全量配色，缺省透明黑会很难看） */
static const struct nk_color GUI_THEME_TABLE[NK_COLOR_COUNT]
    = { /* 文字与底色 */
        [NK_COLOR_TEXT] = { 216, 222, 233, 255 },
        [NK_COLOR_WINDOW] = { 28, 30, 36, 255 },
        [NK_COLOR_HEADER] = { 37, 40, 50, 255 },
        [NK_COLOR_BORDER] = { 58, 63, 75, 255 },
        /* 按钮 */
        [NK_COLOR_BUTTON] = { 43, 48, 59, 255 },
        [NK_COLOR_BUTTON_HOVER] = { 52, 59, 72, 255 },
        [NK_COLOR_BUTTON_ACTIVE] = { 76, 90, 110, 255 },
        /* 勾选框 */
        [NK_COLOR_TOGGLE] = { 43, 48, 59, 255 },
        [NK_COLOR_TOGGLE_HOVER] = { 52, 59, 72, 255 },
        [NK_COLOR_TOGGLE_CURSOR] = { 134, 216, 142, 255 },
        /* 选中 */
        [NK_COLOR_SELECT] = { 37, 40, 50, 255 },
        [NK_COLOR_SELECT_ACTIVE] = { 76, 90, 110, 255 },
        /* 滑块 */
        [NK_COLOR_SLIDER] = { 37, 40, 50, 255 },
        [NK_COLOR_SLIDER_CURSOR] = { 134, 216, 142, 255 },
        [NK_COLOR_SLIDER_CURSOR_HOVER] = { 154, 226, 162, 255 },
        [NK_COLOR_SLIDER_CURSOR_ACTIVE] = { 114, 196, 122, 255 },
        /* 数值属性 */
        [NK_COLOR_PROPERTY] = { 43, 48, 59, 255 },
        /* 输入框 */
        [NK_COLOR_EDIT] = { 43, 48, 59, 255 },
        [NK_COLOR_EDIT_CURSOR] = { 216, 222, 233, 255 },
        /* 下拉框 */
        [NK_COLOR_COMBO] = { 43, 48, 59, 255 },
        /* 图表（未用到） */
        [NK_COLOR_CHART] = { 43, 48, 59, 255 },
        [NK_COLOR_CHART_COLOR] = { 134, 216, 142, 255 },
        [NK_COLOR_CHART_COLOR_HIGHLIGHT] = { 154, 226, 162, 255 },
        /* 滚动条 */
        [NK_COLOR_SCROLLBAR] = { 28, 30, 36, 255 },
        [NK_COLOR_SCROLLBAR_CURSOR] = { 76, 90, 110, 255 },
        [NK_COLOR_SCROLLBAR_CURSOR_HOVER] = { 96, 112, 134, 255 },
        [NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = { 116, 134, 158, 255 },
        /* 树节点表头 */
        [NK_COLOR_TAB_HEADER] = { 37, 40, 50, 255 },
        /* 旋钮（未用到） */
        [NK_COLOR_KNOB] = { 43, 48, 59, 255 },
        [NK_COLOR_KNOB_CURSOR] = { 134, 216, 142, 255 },
        [NK_COLOR_KNOB_CURSOR_HOVER] = { 154, 226, 162, 255 },
        [NK_COLOR_KNOB_CURSOR_ACTIVE] = { 114, 196, 122, 255 } };

static void gui_sync_color_from_config(AppContext* app)
{
    app->gui.temp_nk_bg = app->gui.nk_bg = u32_to_nk(app->config.color_bg);
    app->gui.temp_nk_fg = app->gui.nk_fg = u32_to_nk(app->config.color_fg);
    color_to_hex_str(app->gui.nk_bg, app->gui.hex_bg);
    color_to_hex_str(app->gui.nk_fg, app->gui.hex_fg);
}
static void gui_sync_color_instantly(AppContext* app)
{
    app->config.color_bg = nk_to_u32(app->gui.nk_bg);
    app->config.color_fg = nk_to_u32(app->gui.nk_fg);
}
static void gui_sync_color_from_hex(AppContext* app)
{
    app->gui.nk_bg = hex_str_to_color(app->gui.hex_bg);
    app->gui.nk_fg = hex_str_to_color(app->gui.hex_fg);
    gui_sync_color_instantly(app);
}

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
    /* 界面为纯英文，Nuklear 默认字形范围（ASCII + 拉丁扩展 0x0020–0x024F）即可全覆盖，
       无需自定义 range，图集最小、烘焙最快 */
    struct nk_font* default_font = NULL;
    const char* font_candidates[] = { "C:\\Windows\\Fonts\\consola.ttf", // Consolas 等宽
                                      "C:\\Windows\\Fonts\\cour.ttf" }; // Courier New
    struct nk_font_config cfg = nk_font_config(gui_ui_scale(display, 16));
    for (size_t i = 0; i < sizeof(font_candidates) / sizeof(font_candidates[0]); i++) {
        FILE* fp = fopen(font_candidates[i], "rb");
        if (fp != NULL) {
            fclose(fp);
            default_font = nk_font_atlas_add_from_file(atlas, font_candidates[i], gui_ui_scale(display, 16), &cfg);
            break;
        }
    }
    if (default_font == NULL) {
        default_font = nk_font_atlas_add_default(atlas, gui_ui_scale(display, 16), 0);
    }
    nk_sdl_font_stash_end();

    if (default_font) {
        nk_style_set_font(ctx, &default_font->handle);
    }
    nk_style_from_table(ctx, GUI_THEME_TABLE); // 应用深色主题
    gui->is_fullscreen = false;
    gui->show_debugger = true;
    gui->show_keymap_help = false;
    gui->show_rom_library = false;
    gui->show_settings = false;
    gui->curr_theme_index = (ColorTheme)app->config.theme_type; // 与持久化主题一致
    gui_sync_color_from_config(app);

    memset(gui->popups_active, 0, sizeof(gui->popups_active));
    memset(gui->popups_changed, 0, sizeof(gui->popups_changed));
    return ctx;
}

static nk_bool nk_checkbox_label_bool(nk_ctx_t* ctx, const char* label, bool* active)
{
    nk_bool temp = (int)*active;
    nk_bool ret = nk_checkbox_label(ctx, label, &temp);
    *active = (bool)temp;
    return ret;
}

/* 文件对话框 / ROM 目录浏览器的加载包装：失败时弹错误框 */
static bool load_rom_file(AppContext* app, const char* path)
{
    if (!app_load_rom(app, path)) {
        tinyfd_messageBox("Error", "Failed to load ROM file!", "ok", "error", 1);
        return false;
    }
    return true;
}

static void handle_rom_loading(AppContext* app)
{
    const char* fillter_patherns[] = { "*.ch8", "*.rom", "*.bin" };
    const char* selected_path
        = tinyfd_openFileDialog("Select CHIP-8 ROM file", "", 3, fillter_patherns, "CHIP-8 ROM file (.ch8, .rom)", 0);

    if (selected_path) { // 用户取消对话框不是错误
        load_rom_file(app, selected_path);
    }
}

static const char* rom_name_only(const char* path)
{
    const char* slash = strrchr(path, '/');
    const char* bslash = strrchr(path, '\\');
    const char* base = bslash;
    if (slash && (base == NULL || slash > base)) base = slash;
    return base ? base + 1 : path;
}

/* 扫描 rom_dir 下的 *.ch8 / *.rom / *.bin 文件 */
static void scan_rom_dir(AppContext* app)
{
    GuiState* gui = &app->gui;
    gui->rom_count = 0;
    DIR* dir = opendir(gui->rom_dir);
    if (dir == NULL) return;
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL && gui->rom_count < 64) {
        if (ent->d_name[0] == '.') continue;
        const char* dot = strrchr(ent->d_name, '.');
        if (dot == NULL) continue;
        if (SDL_strcasecmp(dot, ".ch8") != 0 && SDL_strcasecmp(dot, ".rom") != 0 && SDL_strcasecmp(dot, ".bin") != 0) {
            continue;
        }
        char full[256];
        size_t dir_len = strlen(gui->rom_dir);
        size_t name_len = strlen(ent->d_name);
        if (dir_len + name_len + 2 > sizeof(full)) continue; // 路径太长放不下
        memcpy(full, gui->rom_dir, dir_len);
        full[dir_len] = '\\';
        memcpy(full + dir_len + 1, ent->d_name, name_len);
        full[dir_len + 1 + name_len] = '\0';
        struct stat st;
        if (stat(full, &st) != 0 || !S_ISREG(st.st_mode)) continue;
        strncpy(gui->rom_files[gui->rom_count], full, sizeof(gui->rom_files[gui->rom_count]) - 1);
        gui->rom_files[gui->rom_count][sizeof(gui->rom_files[gui->rom_count]) - 1] = '\0';
        gui->rom_count++;
    }
    closedir(dir);
}

static void handle_romdir_show(AppContext* app)
{
    app->gui.popups_active[POPUP_ROMLIB] = true;
}
static nk_bool nk_filter_sharp_hex(const struct nk_text_edit* box, nk_rune unicode)
{
    if (box->string.len >= 7 && box->select_start == box->select_end) {
        return nk_false;
    }
    if (unicode == '#') {
        return ((nk_bool)(box->string.len == 0));
    }
    if ((unicode < '0' || unicode > '9') && (unicode < 'a' || unicode > 'f') && (unicode < 'A' || unicode > 'F')) {
        return nk_false;
    }
    return nk_true;
}
static void gui_render_debugger(nk_ctx_t* ctx, AppContext* app)
{
    if (!app->gui.show_debugger) return;
    Window* display = &app->display;
    int win_w = display->window_w;
    int win_h = display->window_h;
    int sidebar_w = SIDEBAR_WIDTH(display);
    int menu_h = MENU_HEIGHT(display);

    struct nk_rect bounds = nk_rect(win_w - sidebar_w, menu_h, sidebar_w, win_h - menu_h);
    Chip8* chip8 = &app->chip8;
    if (nk_begin(ctx, "Debugger", bounds, NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 2);
        nk_label(ctx, "CPU Speed (Hz)", NK_TEXT_LEFT);
        if (nk_property_int(ctx, "", 100, &app->cpu_speed, 10000, 10, 100)) app->cpu_speed_change = true;
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
        if (nk_slider_int(ctx, 100, &app->cpu_speed, 10000, 10)) app->cpu_speed_change = true;
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 2);
        nk_labelf(ctx, NK_TEXT_LEFT, "IPS: %d", app->last_measured_ips);
        nk_labelf(ctx, NK_TEXT_LEFT, "IPF: %.2f", app->ins_per_frame);

        if (nk_tree_push(ctx, NK_TREE_TAB, "Color Theme", NK_MAXIMIZED)) {

            struct nk_colorf temp_cf_fg = nk_color_cf(app->gui.nk_fg);
            struct nk_colorf temp_cf_bg = nk_color_cf(app->gui.nk_bg);
            nk_layout_row_template_begin(ctx, 25);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, gui_ui_scale(display, 35));
            nk_layout_row_template_push_static(ctx, gui_ui_scale(display, 75));
            nk_layout_row_template_end(ctx);
            nk_label(ctx, "Foreground", NK_TEXT_LEFT);
            nk_label(ctx, "hex:", NK_TEXT_LEFT);
            nk_flags state_fg = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, app->gui.hex_fg,
                                                               8, nk_filter_sharp_hex);
            if (state_fg & NK_EDIT_COMMITED) {
                gui_sync_color_from_hex(app);
            }
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 120), 1);
            if (nk_color_pick(ctx, &temp_cf_fg, NK_RGB)) {
                app->gui.nk_fg = nk_rgb_cf(temp_cf_fg);
                gui_sync_color_instantly(app);
            }

            nk_layout_row_template_begin(ctx, 25);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, gui_ui_scale(display, 35));
            nk_layout_row_template_push_static(ctx, gui_ui_scale(display, 75));
            nk_layout_row_template_end(ctx);
            nk_label(ctx, "Background", NK_TEXT_LEFT);
            nk_label(ctx, "hex:", NK_TEXT_LEFT);
            nk_flags state_bg = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, app->gui.hex_bg,
                                                               8, nk_filter_sharp_hex);
            if (state_bg & NK_EDIT_COMMITED) {
                gui_sync_color_instantly(app);
            }
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 120), 1);
            if (nk_color_pick(ctx, &temp_cf_bg, NK_RGB)) {
                app->gui.nk_bg = nk_rgb_cf(temp_cf_bg);
                gui_sync_color_instantly(app);
            }
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Core Register", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 20), 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "PC: 0x%04x", chip8->pc);
            nk_labelf(ctx, NK_TEXT_LEFT, "I: 0x%04x", chip8->I);
            nk_labelf(ctx, NK_TEXT_LEFT, "SP: 0x%02x", chip8->sp);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Registers V0-VF", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 20), 2);
            uint8_t* V = chip8->V;
            for (int i = 0; i < 16; i++) {
                nk_labelf(ctx, NK_TEXT_LEFT, "V%X: %02x", i, V[i]);
            }
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Timers", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 20), 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Delay: %d", chip8->delay_timer);
            nk_labelf(ctx, NK_TEXT_LEFT, "Sound: %d", chip8->sound_timer);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Current Instruction", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
            if (chip8->pc < 4095) {
                uint16_t opcode = (chip8->memory[chip8->pc] << 8) | (chip8->memory[chip8->pc + 1]);
                nk_labelf(ctx, NK_TEXT_LEFT, "[0x%04x] -> 0x%04x", chip8->pc, opcode);
            } else {
                nk_label(ctx, "PC out of bounds, no current instruction", NK_TEXT_LEFT);
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
}
static inline void top_menubor_render(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    Appstate app_state = app->state;
    int win_w = app->display.window_w;

    if (nk_begin(ctx, "MenuBar", nk_rect(0, 0, win_w, MENU_HEIGHT(display)), NK_WINDOW_NO_SCROLLBAR)) {

        nk_menubar_begin(ctx);
        nk_layout_row_static(ctx, MENU_HEIGHT(display), gui_ui_scale(display, 70), 5);

        if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(gui_ui_scale(display, 120), gui_ui_scale(display, 130)))) { /*  file */
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);

            if (nk_menu_item_label(ctx, "Open ROM", NK_TEXT_LEFT)) { // open rom
                handle_rom_loading(app);
            }
            if (nk_menu_item_label(ctx, "Load Directory", NK_TEXT_LEFT)) { // load dir
                handle_romdir_show(app);
            }
            if (!app->rom_loaded) nk_widget_disable_begin(ctx);
            if (nk_menu_item_label(ctx, "Close ROM", NK_TEXT_LEFT)) { // close
                app_request_unload_rom(app);
            }
            if (!app->rom_loaded) nk_widget_disable_end(ctx);

            if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT)) { // exit
                app->is_running = false;
            }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "Emulate", NK_TEXT_LEFT, nk_vec2(gui_ui_scale(display, 120), gui_ui_scale(display, 160)))) { /* emulator */
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);

            if (app_state == STATE_IDLE) nk_widget_disable_begin(ctx);
            if (nk_menu_item_label(ctx, app->state == STATE_PAUSED ? "Resume" : "Pause",
                                   NK_TEXT_LEFT)) { // pause / resuame
                app_pause_or_resume(app);
            }
            if (app_state == STATE_IDLE) nk_widget_disable_end(ctx);

            if (!app->rom_loaded) nk_widget_disable_begin(ctx);
            if (nk_menu_item_label(ctx, "Restart", NK_TEXT_LEFT)) { // restart
                chip8_restart(&app->chip8);
                app->state = STATE_RUNNING; // EXIT 后也能从菜单重启
            }
            if (!app->rom_loaded) nk_widget_disable_end(ctx);
            if (nk_menu_item_label(ctx, "Speed", NK_TEXT_LEFT)) {
                app->gui.popups_active[POPUP_CPU] = true;
                app->gui.popups_changed[POPUP_CPU] = false;
                app->gui.temp_cpu_speed = app->cpu_speed;
            }
            if (nk_menu_item_label(ctx, "Quirks", NK_TEXT_LEFT)) {
                app->gui.popups_active[POPUP_QUIRKS] = true;
            }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "Video", NK_TEXT_LEFT, nk_vec2(gui_ui_scale(display, 120), gui_ui_scale(display, 100)))) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
            if (nk_menu_item_label(ctx, "Toggle Fullscreen", NK_TEXT_LEFT)) {
                app_toggle_fullscreen(app);
            }
            if (nk_menu_item_label(ctx, "Color Theme", NK_TEXT_LEFT)) {
                app->gui.popups_active[POPUP_THEME] = true;
                app->gui.popups_changed[POPUP_THEME] = false;
                app->gui.temp_theme_index = app->gui.curr_theme_index;
            }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "View", NK_TEXT_LEFT, nk_vec2(gui_ui_scale(display, 120), gui_ui_scale(display, 100)))) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
            nk_checkbox_label_bool(ctx, "Show Debugger", &app->gui.show_debugger);

            if (nk_menu_item_label(ctx, "Memory Map", NK_TEXT_LEFT)) {
                app->gui.popups_active[POPUP_MEMORY] = true;
            }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(gui_ui_scale(display, 120), gui_ui_scale(display, 70)))) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
            if (nk_menu_item_label(ctx, "Keymap", NK_TEXT_LEFT)) {
                app->gui.popups_active[POPUP_KEYMAP] = true;
            }

            if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT)) {
                app->gui.popups_active[POPUP_ABOUT] = true;
            }

            nk_menu_end(ctx);
        }
        nk_menubar_end(ctx);
    }
    nk_end(ctx);
}

static inline struct nk_rect nk_get_centre_rect(AppContext* app, int rect_w, int rect_h)
{
    int win_w = app->display.window_w;
    int win_h = app->display.window_h;
    return nk_rect((win_w - rect_w) / 2.0, (win_h - rect_h) / 2.0, rect_w, rect_h);
}

static void gui_render_speed_popup(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
    if (nk_property_int(ctx, "CPU Speed (Hz)", 100, &app->gui.temp_cpu_speed, 10000, 10, 100))
        app->gui.popups_changed[POPUP_CPU] = true;
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 20), 1);
    if (nk_slider_int(ctx, 100, &app->gui.temp_cpu_speed, 10000, 10)) app->gui.popups_changed[POPUP_CPU] = true;

    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 20), 4);
    if (nk_button_label(ctx, "500")) {
        app->gui.temp_cpu_speed = 500;
        app->gui.popups_changed[POPUP_CPU] = true;
    }
    if (nk_button_label(ctx, "700")) {
        app->gui.temp_cpu_speed = 700;
        app->gui.popups_changed[POPUP_CPU] = true;
    }
    if (nk_button_label(ctx, "1000")) {
        app->gui.temp_cpu_speed = 1000;
        app->gui.popups_changed[POPUP_CPU] = true;
    }
    if (nk_button_label(ctx, "2000")) {
        app->gui.temp_cpu_speed = 2000;
        app->gui.popups_changed[POPUP_CPU] = true;
    }
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 60), 1);
    nk_spacer(ctx);

    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 2);
    if (!app->gui.popups_changed[POPUP_CPU]) nk_widget_disable_begin(ctx);
    if (nk_button_label(ctx, "Apply")) {
        if (app->gui.popups_changed[POPUP_CPU]) {
            app->cpu_speed = app->gui.temp_cpu_speed;
            app->cpu_speed_change = true;
        }
        app->gui.popups_active[POPUP_CPU] = false;
    }
    if (!app->gui.popups_changed[POPUP_CPU]) nk_widget_disable_end(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_CPU] = false;
    }
}

static void gui_render_quirks_popup(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    if (nk_tree_push(ctx, NK_TREE_TAB, "Quirks Profiles", NK_MAXIMIZED)) {
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 3);
        if (nk_button_label(ctx, "Original")) {
            chip8_load_quirks(&app->chip8, QUIRK_PROFILE_COSMAC_VIP);
            app->config.quirk_mode = QUIRK_PROFILE_COSMAC_VIP; // 随配置持久化
        }
        if (nk_button_label(ctx, "SCHIP")) {
            chip8_load_quirks(&app->chip8, QUIRK_PROFILE_SCHIP_LEGACY);
            app->config.quirk_mode = QUIRK_PROFILE_SCHIP_LEGACY;
        }
        if (nk_button_label(ctx, "Modern")) {
            chip8_load_quirks(&app->chip8, QUIRK_PROFILE_MODERN);
            app->config.quirk_mode = QUIRK_PROFILE_MODERN;
        }
        nk_tree_pop(ctx);
    }
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
    nk_checkbox_label_bool(ctx, "Clip Quirk (DXYN Clipping)", &app->chip8.clip_quirk);
    nk_checkbox_label_bool(ctx, "Shift Quirk (8XY6/E use Vy)", &app->chip8.shift_quirk);
    nk_checkbox_label_bool(ctx, "Jump Quirk (BNNN uses BxNN)", &app->chip8.jump_quirk);
    nk_checkbox_label_bool(ctx, "VF Reset Quirk (Logic ops reset VF)", &app->chip8.vf_reset_quirk);
    nk_checkbox_label_bool(ctx, "LoadStore Quirk (I Increment)", &app->chip8.loadstore_quirk);
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 30), 1);
    nk_spacer(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_QUIRKS] = false;
    }
}
static void gui_render_theme_popup(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    const char* selcted_theme_name
        = app->gui.temp_theme_index >= THEME_COUNT ? "Custom" : THEME_TABLE[app->gui.temp_theme_index].name;
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
    if (nk_combo_begin_label(ctx, selcted_theme_name, nk_vec2(nk_widget_width(ctx), gui_ui_scale(display, 200)))) {
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
        for (int i = 0; i < THEME_COUNT; i++) {
            if (nk_combo_item_label(ctx, THEME_TABLE[i].name, NK_TEXT_LEFT)) {
                app->gui.temp_theme_index = (ColorTheme)i;
                app->gui.popups_changed[POPUP_THEME] = true;
            }
        }
        nk_combo_end(ctx);
    }
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 40), 1);
    nk_spacer(ctx);

    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 2);
    if (!app->gui.popups_changed[POPUP_THEME]) nk_widget_disable_begin(ctx);
    if (nk_button_label(ctx, "Apply")) {
        app->gui.curr_theme_index = app->gui.temp_theme_index;
        config_apply_theme(&app->config, app->gui.curr_theme_index);
        gui_sync_color_from_config(app);
        app->gui.popups_active[POPUP_THEME] = false;
    }
    if (!app->gui.popups_changed[POPUP_THEME]) nk_widget_disable_end(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_THEME] = false;
        app->gui.popups_changed[POPUP_THEME] = false;
    }
}
/* 默认按键布局（与 system.c 的 app_update_keypad_default 保持一致） */
static const SDL_Keycode DEFAULT_KEYMAP[16]
    = { SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a,
        SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v };

static void gui_render_romlib_popup(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 2);
    if (nk_button_label(ctx, "Choose Directory")) {
        const char* dir
            = tinyfd_selectFolderDialog("Select ROM Directory", app->gui.rom_dir[0] ? app->gui.rom_dir : NULL);
        if (dir) {
            snprintf(app->gui.rom_dir, sizeof(app->gui.rom_dir), "%s", dir);
            scan_rom_dir(app);
        }
    }
    if (nk_button_label(ctx, "Refresh")) {
        if (app->gui.rom_dir[0]) scan_rom_dir(app);
    }

    if (app->gui.rom_count <= 0) {
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 20), 1);
        nk_label(ctx, "Select a directory to list", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 20), 1);
        nk_label(ctx, "*.ch8, *.rom, *.bin files", NK_TEXT_LEFT);
    } else {
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 220), 1);
        if (nk_group_begin(ctx, "romlist", NK_WINDOW_BORDER)) {
            nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
            for (int i = 0; i < app->gui.rom_count; i++) {
                if (nk_button_label(ctx, rom_name_only(app->gui.rom_files[i]))) {
                    if (load_rom_file(app, app->gui.rom_files[i])) {
                        app->gui.popups_active[POPUP_ROMLIB] = false;
                    }
                }
            }
            nk_group_end(ctx);
        }
    }
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 30), 1);
    nk_spacer(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_ROMLIB] = false;
    }
}

static void gui_render_keymap_popup(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    const char* chip8_hex_chars[]
        = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 2);
    nk_label(ctx, "CHIP-8 Key", NK_TEXT_LEFT);
    nk_label(ctx, "PC Keyboard", NK_TEXT_LEFT);
    for (int i = 0; i < 16; i++) {
        SDL_Keycode kc = app->config.keymap_is_default ? DEFAULT_KEYMAP[i] : app->config.custom_keymap[i];
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 2);
        nk_labelf(ctx, NK_TEXT_LEFT, "Key %s", chip8_hex_chars[i]);
        nk_label(ctx, SDL_GetKeyName(kc), NK_TEXT_LEFT);
    }
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 30), 1);
    nk_spacer(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_KEYMAP] = false;
    }
}

static void gui_render_about_popup(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 25), 1);
    nk_label(ctx, "CHIP-8 Emulator", NK_TEXT_CENTERED);
    nk_label(ctx, "C23 + SDL2 + Nuklear immediate-mode GUI", NK_TEXT_CENTERED);
    nk_label(ctx, "SCHIP extension, Quirks profiles, custom themes", NK_TEXT_CENTERED);
    nk_label(ctx, "F4 Pause / F5 Restart", NK_TEXT_CENTERED);
    nk_label(ctx, "F11 Fullscreen / F3 Debugger", NK_TEXT_CENTERED);
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 30), 1);
    nk_spacer(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_ABOUT] = false;
    }
}

static void gui_render_memory_popup(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 340), 1);
    if (nk_group_begin(ctx, "memdump", NK_WINDOW_BORDER)) {
        Chip8* chip8 = &app->chip8;
        nk_layout_row_dynamic(ctx, gui_ui_scale(display, 18), 1);
        for (int addr = 0; addr < 4096; addr += 16) {
            char line[64];
            char* p = line;
            p += sprintf(p, "%04X: ", addr);
            for (int i = 0; i < 16; i++) {
                p += sprintf(p, "%02X ", chip8->memory[addr + i]);
            }
            nk_label(ctx, line, NK_TEXT_LEFT);
        }
        nk_group_end(ctx);
    }
    nk_layout_row_dynamic(ctx, gui_ui_scale(display, 30), 1);
    nk_spacer(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_MEMORY] = false;
    }
}

static const PopupContrl POPUP_REGISTER[POPUP_COUNT]
    = { [POPUP_THEME] = { "Theme Settings", 300, 160, gui_render_theme_popup },
        [POPUP_CPU] = { "CPU Settings", 320, 220, gui_render_speed_popup },
        [POPUP_QUIRKS] = { "Quirks Settings", 400, 320, gui_render_quirks_popup },
        [POPUP_ROMLIB] = { "ROM Library", 400, 320, gui_render_romlib_popup },
        [POPUP_KEYMAP] = { "Keymap", 260, 480, gui_render_keymap_popup },
        [POPUP_ABOUT] = { "About", 380, 240, gui_render_about_popup },
        [POPUP_MEMORY] = { "Memory Map", 480, 430, gui_render_memory_popup } };

static inline void gui_render_popups(nk_ctx_t* ctx, AppContext* app)
{
    Window* display = &app->display;
    for (int i = 0; i < POPUP_COUNT; i++) {
        if (!app->gui.popups_active[i]) continue;
        const PopupContrl* p = &POPUP_REGISTER[i];
        if (nk_begin(ctx, p->title,
                     nk_get_centre_rect(app, gui_ui_scale(display, p->width), gui_ui_scale(display, p->height)),
                     NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
            p->render(ctx, app);
        } else {
            app->gui.popups_active[i] = false;
        }
        nk_end(ctx);
    }
}
void gui_render(AppContext* app)
{
    nk_ctx_t* ctx = app->ctx;
    gui_render_debugger(ctx, app);
    top_menubor_render(ctx, app);
    gui_render_popups(ctx, app);
}