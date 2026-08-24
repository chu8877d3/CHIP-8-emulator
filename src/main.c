#include "system.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    AppContext app = { 0 };
    AppContext* app_p = &app;
    app_init(app_p);

    if (argc > 1) { // 命令行/双击打开：Chip8Emulator.exe <rom.ch8>
        if (!app_load_rom(app_p, argv[1])) {
            fprintf(stderr, "Failed to load ROM: %s\n", argv[1]);
        }
    }

    while (app.is_running) {
        app_handle_events(app_p);
        app_run(app_p);
    }
    app_exit(app_p);
    return 0;
}