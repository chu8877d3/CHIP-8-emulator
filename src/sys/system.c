#include "system.h"

const int TIME_FREQUENCY_HZ = 60;

void app_init(AppContext* app)
{
    app->config = config_init();
    app->is_running = true;
    app->state = STATE_IDLE;
    chip8_init(&app->chip8);
    app->cpu_speed = app->config.cpu_frequency_hz;
    if (!window_init(&app->display, "CHIP-8 emulator", 128, 64, 10)) {
        exit(1);
    }
    app->ctx = gui_init(app);
    app_game_init(app);
}
static inline void app_update_kepad_default(AppContext* app)
{
    const Uint8* state = SDL_GetKeyboardState(NULL);
    bool* keypad = app->chip8.keypad;

    keypad[0x1] = state[SDL_SCANCODE_1];
    keypad[0x2] = state[SDL_SCANCODE_2];
    keypad[0x3] = state[SDL_SCANCODE_3];
    keypad[0xC] = state[SDL_SCANCODE_4];

    keypad[0x4] = state[SDL_SCANCODE_Q];
    keypad[0x5] = state[SDL_SCANCODE_W];
    keypad[0x6] = state[SDL_SCANCODE_E];
    keypad[0xD] = state[SDL_SCANCODE_R];

    keypad[0x7] = state[SDL_SCANCODE_A];
    keypad[0x8] = state[SDL_SCANCODE_S];
    keypad[0x9] = state[SDL_SCANCODE_D];
    keypad[0xE] = state[SDL_SCANCODE_F];

    keypad[0xA] = state[SDL_SCANCODE_Z];
    keypad[0x0] = state[SDL_SCANCODE_X];
    keypad[0xB] = state[SDL_SCANCODE_C];
    keypad[0xF] = state[SDL_SCANCODE_V];
}
void app_handle_events(AppContext* app)
{
    SDL_Event event;
    nk_ctx_t* ctx = app->ctx;
    AppConfig config = app->config;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&event)) {
        nk_sdl_handle_event(&event);

        switch (event.type) {
            case SDL_QUIT:
                app->is_running = false;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED
                    || event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    app->display.window_w = event.window.data1;
                    app->display.window_h = event.window.data2;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (nk_window_is_any_hovered(ctx) || nk_item_is_any_active(ctx)) {
                    break;
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat != 0) break;
                SDL_Keycode key_input = event.key.keysym.sym;
                if (key_input == config.key_fullscreen) {
                    app_toggle_fullscreen(app);
                } else if (key_input == config.key_pause) {
                    app_pause_or_resume(app);
                } else if (key_input == config.key_restart) {
                    chip8_restart(&app->chip8);
                }
        }
    }
    nk_input_end(ctx);
    if (!nk_window_is_any_hovered(ctx)) {
        app_update_kepad_default(app);
    } else {
        memset(app->chip8.keypad, 0, sizeof((app->chip8.keypad)));
    }
}
void app_game_init(AppContext* app)
{
    app->ins_per_frame = (double)app->cpu_speed / (double)TIME_FREQUENCY_HZ;
    app->fixed_dt = 1.0 / (double)TIME_FREQUENCY_HZ;
    app->prev_counter = SDL_GetPerformanceCounter();
    app->ins_accumulator = 0.0;
    app->accumlator = 0.0;
    app->perf_frequency = (double)SDL_GetPerformanceFrequency();
}
void app_game_update(AppContext* app)
{
    app->ins_per_frame = (double)app->cpu_speed / (double)TIME_FREQUENCY_HZ;
}
void app_game_run(AppContext* app)
{
    app->curr_counter = SDL_GetPerformanceCounter();
    app->frametime = (double)(app->curr_counter - app->prev_counter) / app->perf_frequency;
    app->prev_counter = app->curr_counter;

    app->frametime = app->frametime > 0.1 ? 0.1 : app->frametime;
    app->accumlator += app->frametime;
    app->ips_timer += app->state == STATE_RUNNING ? app->frametime : 0;

    Chip8* chip8 = &app->chip8;

    while (app->accumlator >= app->fixed_dt) {
        if (app->state == STATE_RUNNING) {
            if (!chip8->running) {
                app->state = STATE_IDLE;
                break;
            }
            app->ins_accumulator += app->ins_per_frame;
            int ins_to_run = (int)app->ins_accumulator;

            for (int i = 0; i < ins_to_run; i++) {
                chip8_cycle(chip8);
                app->total_ins_count++;
            }

            app->ins_accumulator -= (double)ins_to_run;
            chip8_update_timers(chip8);
        }
        app->accumlator -= app->fixed_dt;
    }
    if (app->ips_timer >= 1.0 && app->state == STATE_RUNNING) {
        app->last_measured_ips = (uint32_t)((double)app->total_ins_count / app->ips_timer);
        app->total_ins_count = 0;
        app->ips_timer -= 1.0;
    }
    bool should_beep = (app->state == STATE_RUNNING && chip8->sound_timer > 0);
    window_play_sound(&app->display, should_beep);
}
void app_run(AppContext* app)
{
    if (app->cpu_speed_change) {
        app_game_update(app);
        app->cpu_speed_change = false;
    }
    app_game_run(app);
    render_window(app);
}
void app_exit(AppContext* app)
{
    window_cleanup(&app->display);
}
