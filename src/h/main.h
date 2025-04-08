#ifndef MINECRAFT_H

#define MC_C_VERSION "Minecraft.c 0.1.2"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "defines.h"

// ---- platform layer ---------------------------------------------------------
#define WIDTH 1280
#define HEIGHT 720
#define MARGIN 20

#define MODE_DEBUG          1
#define MODE_COLLIDE        0
#define MODE_GRAVITY        1
#define MODE_GRAY_BLOCKS    1

// ---- declarations -----------------------------------------------------------
extern f64 delta_time;
extern f64 start_time;
#define dt GetFrameTime()

extern u16 state;
extern u8 state_menu_depth;
extern v3i32 target_coordinates_feet; /*temp*/

// ---- signatures -------------------------------------------------------------
int mc_mkdir(const char *path, u16 mode);
void init_world();

#define MINECRAFT_H
#endif
