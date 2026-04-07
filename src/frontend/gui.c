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
    struct nk_font* default_font = nk_font_atlas_add_default(atlas, 13, 0);
    nk_sdl_font_stash_end();

    if (default_font) {
        nk_style_set_font(ctx, &default_font->handle);
    }
    gui->is_fullscreen = false;
    gui->show_debugger = true;
    gui->show_keymap_help = false;
    gui->show_rom_library = false;
    gui->show_settings = false;
    gui->curr_theme_index = THEME_CLASSIC;
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
    int sidebar_w = SIDEBAR_WIDTH;
    int menu_h = MENU_HEIGHT;

    struct nk_rect bounds = nk_rect(win_w - sidebar_w, menu_h, sidebar_w, win_h - menu_h);
    Chip8* chip8 = &app->chip8;
    if (nk_begin(ctx, "Debugger", bounds, NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 25, 2);
        nk_label(ctx, "CPU Speed (Hz)", NK_TEXT_LEFT);
        if (nk_property_int(ctx, "", 100, &app->cpu_speed, 10000, 10, 100)) app->cpu_speed_change = true;
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_slider_int(ctx, 100, &app->cpu_speed, 10000, 10)) app->cpu_speed_change = true;
        nk_layout_row_dynamic(ctx, 25, 2);
        nk_labelf(ctx, NK_TEXT_LEFT, "IPS: %d", app->last_measured_ips);
        nk_labelf(ctx, NK_TEXT_LEFT, "IPF: %.2f", app->ins_per_frame);

        if (nk_tree_push(ctx, NK_TREE_TAB, "Color Theme", NK_MAXIMIZED)) {

            struct nk_colorf temp_cf_fg = nk_color_cf(app->gui.nk_fg);
            struct nk_colorf temp_cf_bg = nk_color_cf(app->gui.nk_bg);
            nk_layout_row_template_begin(ctx, 25);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 35);
            nk_layout_row_template_push_static(ctx, 75);
            nk_layout_row_template_end(ctx);
            nk_label(ctx, "Foreground", NK_TEXT_LEFT);
            nk_label(ctx, "hex:", NK_TEXT_LEFT);
            nk_flags state_fg = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, app->gui.hex_fg,
                                                               8, nk_filter_sharp_hex);
            if (state_fg & NK_EDIT_COMMITED) {
                gui_sync_color_from_hex(app);
            }
            nk_layout_row_dynamic(ctx, 120, 1);
            if (nk_color_pick(ctx, &temp_cf_fg, NK_RGB)) {
                app->gui.nk_fg = nk_rgb_cf(temp_cf_fg);
                gui_sync_color_instantly(app);
            }

            nk_layout_row_template_begin(ctx, 25);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 35);
            nk_layout_row_template_push_static(ctx, 75);
            nk_layout_row_template_end(ctx);
            nk_label(ctx, "Background", NK_TEXT_LEFT);
            nk_label(ctx, "hex:", NK_TEXT_LEFT);
            nk_flags state_bg = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, app->gui.hex_bg,
                                                               8, nk_filter_sharp_hex);
            if (state_bg & NK_EDIT_COMMITED) {
                gui_sync_color_instantly(app);
            }
            nk_layout_row_dynamic(ctx, 120, 1);
            if (nk_color_pick(ctx, &temp_cf_bg, NK_RGB)) {
                app->gui.nk_bg = nk_rgb_cf(temp_cf_bg);
                gui_sync_color_instantly(app);
            }
            nk_tree_pop(ctx);
        }
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
    }
    nk_end(ctx);
}
static inline void top_menubor_render(nk_ctx_t* ctx, AppContext* app)
{
    Appstate app_state = app->state;
    int win_w = app->display.window_w;

    if (nk_begin(ctx, "MenuBar", nk_rect(0, 0, win_w, MENU_HEIGHT), NK_WINDOW_NO_SCROLLBAR)) {

        nk_menubar_begin(ctx);
        nk_layout_row_static(ctx, MENU_HEIGHT, 70, 5);

        if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 130))) { /*  file */
            nk_layout_row_dynamic(ctx, 25, 1);

            if (nk_menu_item_label(ctx, "Open ROM", NK_TEXT_LEFT)) { // open rom
                handle_rom_loading(app);
            }
            nk_widget_disable_begin(ctx); // 未开发
            if (nk_menu_item_label(ctx, "Load Directory", NK_TEXT_LEFT)) { // load dir
                handle_romdir_show(app);
            }
            nk_widget_disable_end(ctx);
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

        if (nk_menu_begin_label(ctx, "Emulate", NK_TEXT_LEFT, nk_vec2(120, 160))) { /* emulator */
            nk_layout_row_dynamic(ctx, 25, 1);

            if (app_state == STATE_IDLE) nk_widget_disable_begin(ctx);
            if (nk_menu_item_label(ctx, app->state == STATE_PAUSED ? "Resume" : "Pause",
                                   NK_TEXT_LEFT)) { // pause / resuame
                app_pause_or_resume(app);
            }

            if (nk_menu_item_label(ctx, "Restart", NK_TEXT_LEFT)) { // restart
                chip8_restart(&app->chip8);
            }
            if (app_state == STATE_IDLE) nk_widget_disable_end(ctx);
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

        if (nk_menu_begin_label(ctx, "Video", NK_TEXT_LEFT, nk_vec2(120, 130))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_menu_item_label(ctx, "Toggle Fullscreen", NK_TEXT_LEFT)) {
                app_toggle_fullscreen(app);
            }
            if (nk_menu_item_label(ctx, "Size", NK_TEXT_LEFT)) { // 不知道干嘛的
            }
            if (nk_menu_item_label(ctx, "Color Theme", NK_TEXT_LEFT)) {
                app->gui.popups_active[POPUP_THEME] = true;
                app->gui.popups_changed[POPUP_THEME] = false;
                app->gui.temp_theme_index = app->gui.curr_theme_index;
            }
            if (nk_menu_item_label(ctx, "Aspect", NK_TEXT_LEFT)) {
            }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "View", NK_TEXT_LEFT, nk_vec2(120, 100))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_checkbox_label_bool(ctx, "Show Debugger", &app->gui.show_debugger);

            if (nk_menu_item_label(ctx, "Memory Map", NK_TEXT_LEFT)) {
            }
            if (nk_menu_item_label(ctx, "Log", NK_TEXT_LEFT)) {
            }
            nk_menu_end(ctx);
        }

        if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(120, 70))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_menu_item_label(ctx, "Kepmap", NK_TEXT_LEFT)) {
            }

            if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT)) {
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
    nk_layout_row_dynamic(ctx, 25, 1);
    if (nk_property_int(ctx, "CPU Speed (Hz)", 100, &app->gui.temp_cpu_speed, 10000, 10, 100))
        app->gui.popups_changed[POPUP_CPU] = true;
    nk_layout_row_dynamic(ctx, 20, 1);
    if (nk_slider_int(ctx, 100, &app->gui.temp_cpu_speed, 10000, 10)) app->gui.popups_changed[POPUP_CPU] = true;

    nk_layout_row_dynamic(ctx, 20, 4);
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
    nk_layout_row_dynamic(ctx, 60, 1);
    nk_spacer(ctx);

    nk_layout_row_dynamic(ctx, 25, 2);
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
    if (nk_tree_push(ctx, NK_TREE_TAB, "Quirks Profiles", NK_MAXIMIZED)) {
        nk_layout_row_dynamic(ctx, 25, 3);
        if (nk_button_label(ctx, "Original")) chip8_load_quirks(&app->chip8, QUIRK_PROFILE_COSMAC_VIP);
        if (nk_button_label(ctx, "SCHIP")) chip8_load_quirks(&app->chip8, QUIRK_PROFILE_SCHIP_LEGACY);
        if (nk_button_label(ctx, "Modern")) chip8_load_quirks(&app->chip8, QUIRK_PROFILE_MODERN);
        nk_tree_pop(ctx);
    }
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_checkbox_label_bool(ctx, "Clip Quirk (DXYN Clipping)", &app->chip8.clip_quirk);
    nk_checkbox_label_bool(ctx, "Shift Quirk (8XY6/E use Vy)", &app->chip8.shift_quirk);
    nk_checkbox_label_bool(ctx, "Jump Quirk (BNNN uses BxNN)", &app->chip8.jump_quirk);
    nk_checkbox_label_bool(ctx, "VF Reset Quirk (Logic ops reset VF)", &app->chip8.vf_reset_quirk);
    nk_checkbox_label_bool(ctx, "LoadStore Quirk (I Increment)", &app->chip8.loadstore_quirk);

    nk_layout_row_dynamic(ctx, 30, 1);
    nk_spacer(ctx);
    if (nk_button_label(ctx, "Close")) {
        app->gui.popups_active[POPUP_QUIRKS] = false;
    }
}
static void gui_render_theme_popup(nk_ctx_t* ctx, AppContext* app)
{
    const char* selcted_theme_name
        = app->gui.temp_theme_index == THEME_CUSTOM ? "Custom" : THEME_TABLE[app->gui.temp_theme_index].name;
    nk_layout_row_dynamic(ctx, 25, 1);
    if (nk_combo_begin_label(ctx, selcted_theme_name, nk_vec2(nk_widget_width(ctx), 200))) {
        nk_layout_row_dynamic(ctx, 25, 1);
        for (int i = 0; i < THEME_COUNT; i++) {
            if (nk_combo_item_label(ctx, THEME_TABLE[i].name, NK_TEXT_LEFT)) {
                app->gui.temp_theme_index = (ColorTheme)i;
                app->gui.popups_changed[POPUP_THEME] = true;
            }
        }
        nk_combo_end(ctx);
    }
    nk_layout_row_dynamic(ctx, 40, 1);
    nk_spacer(ctx);

    nk_layout_row_dynamic(ctx, 25, 2);
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
static const PopupContrl POPUP_REGISTER[POPUP_COUNT]
    = { [POPUP_THEME] = { "Theme Settings", 300, 160, gui_render_theme_popup },
        [POPUP_CPU] = { "CPU Settings", 320, 220, gui_render_speed_popup },
        [POPUP_QUIRKS] = { "Quirks Settings", 400, 320, gui_render_quirks_popup } };

static inline void gui_render_popups(nk_ctx_t* ctx, AppContext* app)
{
    for (int i = 0; i < POPUP_COUNT; i++) {
        if (!app->gui.popups_active[i]) continue;
        const PopupContrl* p = &POPUP_REGISTER[i];
        if (nk_begin(ctx, p->title, nk_get_centre_rect(app, p->width, p->height),
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