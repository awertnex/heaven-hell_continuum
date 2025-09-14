#ifndef GAME_H
#define GAME_H

#define GAME_AUTHOR         "Author: Lily Awertnex"
#define GAME_NAME           "Heaven-Hell Continuum"
#define GAME_VERSION        "0.2.0-alpha"

#include "../engine/h/core.h"
#include "../engine/h/defines.h"

/* ---- section: definitions ------------------------------------------------ */

#define MODE_DEBUG          1
#define MODE_COLLIDE        0

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
extern f64 game_start_time;
extern u64 game_tick;
extern u64 game_days;
extern Uniform uniform;

#endif /* GAME_MAIN_H */

