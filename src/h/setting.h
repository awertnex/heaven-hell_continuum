#ifndef MC_C_SETTING_H

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../engine/h/defines.h"

// ---- internal ---------------------------------------------------------------
#define SETTING_CAMERA_DISTANCE_MAX         4.0f
#define SETTING_REACH_DISTANCE_MAX          5.0f
#define SETTING_DAY_TICKS_MAX               24000

// ---- options menu -----------------------------------------------------------
#define SETTING_FOV_DEFAULT                 70.0f
#define SETTING_FOV_MIN                     30.0f
#define SETTING_FOV_MAX                     110.0f
#define SETTING_MOUSE_SENSITIVITY_DEFAULT   120.0f
#define SETTING_MOUSE_SENSITIVITY_MIN       0.0f
#define SETTING_MOUSE_SENSITIVITY_MAX       200.0f

// ---- video ------------------------------------------------------------------
#define SETTING_RENDER_DISTANCE_DEFAULT     3
#define SETTING_RENDER_DISTANCE_MIN         1
#define SETTING_RENDER_DISTANCE_MAX         8
#define SETTING_GUI_SCALE_DEFAULT           2.0f
#define SETTING_GUI_SCALE_0                 // TODO: auto gui scale
#define SETTING_GUI_SCALE_1                 1.0f
#define SETTING_GUI_SCALE_2                 2.0f
#define SETTING_GUI_SCALE_3                 3.0f
#define SETTING_GUI_SCALE_4                 4.0f

// ---- settings ---------------------------------------------------------------
typedef struct settings
{
    // ---- internal -----------------------------------------------------------
    u8 reach_distance;  // for player reach (arm length basically)

    // ---- options menu -------------------------------------------------------
    u16 fov;
    f64 mouse_sensitivity;

    // ---- video --------------------------------------------------------------
    u8 render_distance;
    //TODO: repair
    f32 gui_scale;
} settings;

// ---- declarations -----------------------------------------------------------
extern settings setting;

#define MC_C_SETTING_H
#endif

