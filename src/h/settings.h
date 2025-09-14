#ifndef GAME_SETTING_H
#define GAME_SETTING_H

#include "../engine/h/defines.h"

/* ---- internal ------------------------------------------------------------ */
#define SETTING_CAMERA_DISTANCE_MAX         4.0f
#define SETTING_REACH_DISTANCE_MAX          5.0f
#define SETTING_DAY_TICKS_MAX               24000

/* ---- options menu -------------------------------------------------------- */
#define SETTING_FOV_DEFAULT                 70.0f
#define SETTING_FOV_MIN                     30.0f
#define SETTING_FOV_MAX                     150.0f
#define SETTING_MOUSE_SENSITIVITY_DEFAULT   120.0f
#define SETTING_MOUSE_SENSITIVITY_MIN       0.0f
#define SETTING_MOUSE_SENSITIVITY_MAX       200.0f

/* ---- video --------------------------------------------------------------- */
#define SETTING_RENDER_DISTANCE_DEFAULT     6
#define SETTING_RENDER_DISTANCE_MIN         2
#define SETTING_RENDER_DISTANCE_MAX         16
#define SETTING_TARGET_FPS_DEFAULT          60
#define SETTING_TARGET_FPS_MIN              1
#define SETTING_TARGET_FPS_MAX              256
#define SETTING_GUI_SCALE_DEFAULT           2.0f
#define SETTING_GUI_SCALE_0                 /* TODO: auto gui scale */
#define SETTING_GUI_SCALE_1                 1.0f
#define SETTING_GUI_SCALE_2                 2.0f
#define SETTING_GUI_SCALE_3                 3.0f
#define SETTING_GUI_SCALE_4                 4.0f

/* ---- section: settings --------------------------------------------------- */

typedef struct Settings
{
    /* ---- internal -------------------------------------------------------- */
    u8 reach_distance;  /* for player reach (arm length basically) */

    /* ---- options --------------------------------------------------------- */
    f64 mouse_sensitivity;

    /* ---- video ----------------------------------------------------------- */
    u8 render_distance;
    u32 target_fps;
    f32 gui_scale;
} Settings;

/* ---- section: declarations ----------------------------------------------- */

extern Settings settings;

#endif /* GAME_SETTING_H */

