#ifndef LAUNCHER_H

#include "main.h"
#include "gui.h"
#include "logic.h"

#define LAUNCHER_ENABLED 1
#define COL_LAUNCHER_BG color(0x30, 0x30, 0x30, 0xFF, 0xFF)

// ---- declarations -----------------------------------------------------------
extern v2i16 render_size;

// ---- signatures -------------------------------------------------------------
void init_launcher();
void update_launcher();
u8 close_launcher();
void update_launcher_input();

#define LAUNCHER_H
#endif
