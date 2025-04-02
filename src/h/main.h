#ifndef MINECRAFT_H

#define MC_C_VERSION "Minecraft.c 0.1.2"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include "defines.h"

// ---- from linux/limits.h ----------------------------------------------------
#define NAME_MAX         255	/* # chars in a file name */
#define PATH_MAX        4096	/* # chars in a path name including nul */

#define WIDTH 1280
#define HEIGHT 720
#define MARGIN 20

#define MODE_COLLIDE        0
#define MODE_GRAVITY        1
#define MODE_GRAY_BLOCKS    1

typedef struct Window
{
    v2f32 scl;
} WindowInfo;

// ---- declarations -----------------------------------------------------------
extern WindowInfo win;
extern f64 delta_time;
extern f64 start_time;
#define dt GetFrameTime()

extern u16 state;
extern u8 state_menu_depth;
extern v3i32 target_coordinates_feet; /*temp*/

// ---- signatures -------------------------------------------------------------
void init_world();

#define MINECRAFT_H
#endif
