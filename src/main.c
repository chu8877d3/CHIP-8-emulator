#include "system.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    AppContext app = {0};
    AppContext* app_p = &app;
    app_init(app_p);

    while (app.is_running) {
        app_handle_events(app_p);
        app_run(app_p);
    }
    app_exit(app_p);
    return 0;
}