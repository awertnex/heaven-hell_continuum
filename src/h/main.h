#ifndef MC_C_H
#define MC_C_H

#define MC_C_AUTHOR     "Author: Lily Awertnex"
#define MC_C_NAME       "heaven-hell_continuum"
#define MC_C_VERSION    "0.2.0-alpha"

#include "../engine/h/core.h"
#include "../engine/h/defines.h"

/* ---- section: definitions ------------------------------------------------ */

#define MARGIN 20

#define MODE_DEBUG          1
#define MODE_COLLIDE        0
#define MODE_GRAVITY        1

#define THREAD_COUNT 2 /* TODO: use for multithreading */

typedef struct Uniform
{
    struct /* defaults */
    {
        GLint mat_perspective;
        GLint camera_position;
        GLint sun_rotation;
        GLint sky_color;
    } defaults;

    struct /* skybox */
    {
        GLint camera_position;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint time;
        GLint sun_rotation;
        GLint sky_color;
    } skybox;

    struct /* gizmo */
    {
        GLint render_ratio;
        GLint mat_target;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
    } gizmo;

} Uniform;

/* ---- section: declarations ----------------------------------------------- */

extern Render render;
extern u32 state;
extern u8 state_menu_depth;

extern f64 delta_time;
extern f64 game_start_time;
extern u64 game_tick;
extern u64 game_days;
#define dt (glfwGetTime() - game_start_time)

extern Uniform uniform;

/* ---- section: signatures ------------------------------------------------- */

void init_world(str *str);

#endif /* MC_C_MAIN_H */

