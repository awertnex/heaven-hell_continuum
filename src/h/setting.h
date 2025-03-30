#ifndef SETTING_H
#include <raylib.h>
#include "defines.h"

// ---- internal ---------------------------------------------------------------
#define SETTING_CAMERA_DISTANCE_MAX         4
#define SETTING_REACH_DISTANCE_MAX          5

// ---- options menu -----------------------------------------------------------
#define SETTING_FOV_DEFAULT                 70
#define SETTING_FOV_MIN                     30
#define SETTING_FOV_MAX                     110
#define SETTING_MOUSE_SENSITIVITY_DEFAULT   120
#define SETTING_MOUSE_SENSITIVITY_MIN       0
#define SETTING_MOUSE_SENSITIVITY_MAX       200

// ---- video ------------------------------------------------------------------
#define SETTING_RENDER_DISTANCE_DEFAULT     3
#define SETTING_RENDER_DISTANCE_MIN         2
#define SETTING_RENDER_DISTANCE_MAX         8
#define SETTING_GUI_SCALE_DEFAULT           1.0f
#define SETTING_GUI_SCALE_MIN               1.0f //TODO: figure out values
#define SETTING_GUI_SCALE_MAX               3.0f //TODO: figure out values

// ---- settings ---------------------------------------------------------------
typedef struct settings
{
    // ---- internal -----------------------------------------------------------
    u8 reach_distance;  // for player functionality reach (arm length basically)

    // ---- options menu -------------------------------------------------------
    u16 fov;
    u16 mouse_sensitivity;

    // ---- video --------------------------------------------------------------
    u8 render_distance;
    //TODO: repair
    f32 gui_scale;
} settings;

// ---- declarations -----------------------------------------------------------
extern settings setting;

#define SETTING_H
#endif
