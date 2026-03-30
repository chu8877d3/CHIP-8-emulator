#ifndef RENDERER_H
#define RENDERER_H
#include "chip8.h"
#include "gui.h"
#include "window.h"

typedef struct AppContext AppContext;

void render_game(AppContext* app);
void render_window(AppContext* app);

#endif