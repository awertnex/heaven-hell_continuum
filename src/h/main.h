#ifndef MC_C_H
#define MC_C_H

#define MC_C_VERSION "Minecraft.c 0.1.4"
#define MC_C_AUTHOR "Author: Lily Awertnex"

#include "../engine/h/defines.h"

/* ---- definitions --------------------------------------------------------- */
#define MARGIN 20

#define MODE_DEBUG          1
#define MODE_COLLIDE        0
#define MODE_GRAVITY        1

#define THREAD_COUNT 2 /* TODO: use for multithreading */

/* ---- declarations -------------------------------------------------------- */
extern f64 delta_time;
extern f64 game_start_time;
extern u64 game_tick;
extern u64 game_days;
#define dt GetFrameTime()

extern u16 state;
extern u8 state_menu_depth;

/* ---- signatures ---------------------------------------------------------- */
int mc_c_mkdir(str *path, u16 mode);
void init_world(str *str);

#endif /* MC_C_MAIN_H */

