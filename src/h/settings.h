#ifndef GAME_SETTING_H
#define GAME_SETTING_H

#include "../engine/h/defines.h"

/* ---- internal ------------------------------------------------------------ */
#define SETTING_CAMERA_DISTANCE_MAX         4.0f
#define SETTING_REACH_DISTANCE_MAX          5.0f
#define SETTING_DAY_TICKS_MAX               24000
#define SETTING_LERP_SPEED_DEFAULT          20.0f
#define SETTING_LERP_SPEED_GLIDE            2.5f
#define SETTING_LERP_SPEED_RIGID            100.0f

#define SETTING_PLAYER_EYE_HEIGHT           1.6f
#define SETTING_PLAYER_JUMP_HEIGHT          8.0f
#define SETTING_PLAYER_SPEED_WALK           4.0f
#define SETTING_PLAYER_SPEED_FLY            9.0f
#define SETTING_PLAYER_SPEED_FLY_FAST       40.0f
#define SETTING_PLAYER_SPEED_SNEAK          1.5f
#define SETTING_PLAYER_SPEED_SPRINT         8.0f
#define SETTING_PLAYER_SPEED_MAX            100.0f

#define SETTING_HOTBAR_SLOTS_MAX            10

/* ---- options menu -------------------------------------------------------- */
#define SETTING_FOV_DEFAULT                 70.0f
#define SETTING_FOV_MIN                     30.0f
#define SETTING_FOV_MAX                     150.0f
#define SETTING_MOUSE_SENSITIVITY_DEFAULT   110.0f
#define SETTING_MOUSE_SENSITIVITY_MIN       10.0f
#define SETTING_MOUSE_SENSITIVITY_MAX       200.0f

/* ---- video --------------------------------------------------------------- */
#define SETTING_RENDER_DISTANCE_DEFAULT     6
#define SETTING_RENDER_DISTANCE_MIN         2
#define SETTING_RENDER_DISTANCE_MAX         32
#define SETTING_TARGET_FPS_DEFAULT          60
#define SETTING_TARGET_FPS_MIN              1
#define SETTING_TARGET_FPS_MAX              256
#define SETTING_GUI_SCALE_DEFAULT           2.0f
#define SETTING_GUI_SCALE_0                 /* TODO: auto gui scale */
#define SETTING_GUI_SCALE_1                 1.0f
#define SETTING_GUI_SCALE_2                 2.0f
#define SETTING_GUI_SCALE_3                 3.0f
#define SETTING_GUI_SCALE_4                 4.0f

typedef struct Settings
{
    /* ---- internal -------------------------------------------------------- */
    u8 reach_distance;  /* for player reach (arm length basically) */
    f32 lerp_speed;

    /* ---- options --------------------------------------------------------- */
    f64 mouse_sensitivity;

    /* ---- video ----------------------------------------------------------- */
    u8 render_distance;
    u32 target_fps;
    f32 gui_scale;
} Settings;

extern Settings settings;

#endif /* GAME_SETTING_H */
