#ifndef MINECRAFT_H

#define VERSION "Minecraft.c 0.1.1"

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include <defines.h>
#include "logic.h"

#define WIDTH 1280
#define HEIGHT 720
#define MARGIN 20

enum DebugStates
{
    ModeDebug =     1,
    ModeCollide =   1,
    ModeGravity =   1,
}; /* DebugStates */

typedef struct window
{
    v2f32 scl;
} window;

// ---- declarations -----------------------------------------------------------
extern window win;
extern f64 delta_time;
extern f64 start_time;
//#define dt (get_time_ms() - start_time)
#define dt GetFrameTime()

extern u16 state;
extern u8 state_menu_depth;
extern v3i32 target_coordinates_feet; /*temp*/

// ---- platform ---------------------------------------------------------------
#if defined __linux__
#define PLATFORM_LINUX

#elif defined _WIN32 || defined _WIN64 || defined __CYGWIN__s
#define PLATFORM_WINDOWS

#endif // PLATFORM

#define MINECRAFT_H
#endif
