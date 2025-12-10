#include <engine/h/core.h>
#include <engine/h/types.h>

#include "h/main.h"
#include "h/input.h"

/* ---- movement ------------------------------------------------------------ */

u32 bind_walk_forward =             KEY_W;
u32 bind_walk_backward =            KEY_S;
u32 bind_strafe_left =              KEY_A;
u32 bind_strafe_right =             KEY_D;
u32 bind_jump =                     KEY_SPACE;
u32 bind_sprint =                   KEY_LEFT_SHIFT;
u32 bind_sneak =                    KEY_LEFT_CONTROL;

/* ---- gameplay ------------------------------------------------------------ */

u32 bind_attack_or_destroy =        GLFW_MOUSE_BUTTON_LEFT;
u32 bind_sample_block =             GLFW_MOUSE_BUTTON_MIDDLE;
u32 bind_build_or_use =             GLFW_MOUSE_BUTTON_RIGHT;

/* ---- inventory ----------------------------------------------------------- */

u32 bind_selected_item =            KEY_Q;
u32 bind_hotbar[SET_HOTBAR_SLOTS_MAX] =
{
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
};
u32 bind_hotbar_kp[SET_HOTBAR_SLOTS_MAX] =
{
    KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4, KEY_KP_5,
    KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_0,
};
u32 bind_inventory =                KEY_E;

/* ---- miscellaneous ------------------------------------------------------- */

u32 bind_toggle_hud =               KEY_F1;
u32 bind_take_screenshot =          KEY_F2;
u32 bind_toggle_debug =             KEY_F3;
u32 bind_toggle_cinematic_camera =  KEY_F4;
u32 bind_toggle_perspective =       KEY_F5;
u32 bind_toggle_fullscreen =        KEY_F11;
u32 bind_toggle_zoom =              KEY_Z;
u32 bind_pause =                    KEY_ESCAPE;
u32 bind_chat_or_command =          KEY_SLASH;

/* ---- debug & menu -------------------------------------------------------- */

/* TODO: navigate menus with arrow keys */
u32 bind_left =                     KEY_LEFT;
u32 bind_right =                    KEY_RIGHT;
u32 bind_down =                     KEY_DOWN;
u32 bind_up =                       KEY_UP;
u32 bind_debug_mod =                KEY_LEFT_ALT;
u32 bind_toggle_super_debug =       KEY_TAB;
u32 bind_toggle_trans_blocks =      KEY_T;
u32 bind_toggle_chunk_bounds =      KEY_C;
u32 bind_toggle_bounding_boxes =    KEY_B;
u32 bind_toggle_chunk_gizmo =       KEY_G;
u32 bind_toggle_chunk_queue_visualizer = KEY_V;
