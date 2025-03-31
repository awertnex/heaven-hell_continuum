#ifndef LAUNCHER_H

#define VECTOR2_TYPES
#include "../h/defines.h"

#define MC_C_LAUNCHER_VERSION "Launcher 0.1.0"
#define color(r, g, b, v, a) (Color){((f32)r/255)*v, ((f32)g/255)*v, ((f32)b/255)*v, a}

// ---- colors -----------------------------------------------------------------
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
