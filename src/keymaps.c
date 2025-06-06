#ifndef MC_C_KEYMAPS_H
#define MC_C_KEYMAPS_H

#include "dependencies/raylib-5.5/include/raylib.h"

/* ---- movement ------------------------------------------------------------ */
KeyboardKey bind_jump =                     KEY_SPACE;
KeyboardKey bind_sneak =                    KEY_LEFT_CONTROL;
KeyboardKey bind_sprint =                   KEY_LEFT_SHIFT;
KeyboardKey bind_strafe_left =              KEY_A;
KeyboardKey bind_strafe_right =             KEY_D;
KeyboardKey bind_walk_backwards =           KEY_S;
KeyboardKey bind_walk_forwards =            KEY_W;

/* ---- gameplay ------------------------------------------------------------ */
MouseButton bind_attack_or_destroy =        MOUSE_BUTTON_LEFT;
MouseButton bind_pick_block =               MOUSE_BUTTON_MIDDLE;
MouseButton bind_use_item_or_place_block =  MOUSE_BUTTON_RIGHT;

/* ---- inventory ----------------------------------------------------------- */
KeyboardKey bind_drop_selected_item =       KEY_Q;
KeyboardKey bind_hotbar_slot_1 =            KEY_ONE;
KeyboardKey bind_hotbar_slot_2 =            KEY_TWO;
KeyboardKey bind_hotbar_slot_3 =            KEY_THREE;
KeyboardKey bind_hotbar_slot_4 =            KEY_FOUR;
KeyboardKey bind_hotbar_slot_5 =            KEY_FIVE;
KeyboardKey bind_hotbar_slot_6 =            KEY_SIX;
KeyboardKey bind_hotbar_slot_7 =            KEY_SEVEN;
KeyboardKey bind_hotbar_slot_8 =            KEY_EIGHT;
KeyboardKey bind_hotbar_slot_9 =            KEY_NINE;
KeyboardKey bind_hotbar_slot_0 =            KEY_ZERO;
KeyboardKey bind_hotbar_slot_kp_1 =         KEY_KP_1;
KeyboardKey bind_hotbar_slot_kp_2 =         KEY_KP_2;
KeyboardKey bind_hotbar_slot_kp_3 =         KEY_KP_3;
KeyboardKey bind_hotbar_slot_kp_4 =         KEY_KP_4;
KeyboardKey bind_hotbar_slot_kp_5 =         KEY_KP_5;
KeyboardKey bind_hotbar_slot_kp_6 =         KEY_KP_6;
KeyboardKey bind_hotbar_slot_kp_7 =         KEY_KP_7;
KeyboardKey bind_hotbar_slot_kp_8 =         KEY_KP_8;
KeyboardKey bind_hotbar_slot_kp_9 =         KEY_KP_9;
KeyboardKey bind_hotbar_slot_kp_0 =         KEY_KP_0;
KeyboardKey bind_open_or_close_inventory =  KEY_E;
KeyboardKey bind_swap_item_with_offhand =   KEY_F;

/* ---- miscellaneous ------------------------------------------------------- */
KeyboardKey bind_take_screenshot =          KEY_F2;
KeyboardKey bind_toggle_hud =               KEY_F1;
KeyboardKey bind_toggle_debug =             KEY_F3;
KeyboardKey bind_toggle_cinematic_camera =  KEY_F6;
KeyboardKey bind_toggle_fullscreen =        KEY_F11;
KeyboardKey bind_toggle_perspective =       KEY_F5;
KeyboardKey bind_pause =                    KEY_ESCAPE;
KeyboardKey bind_chat_or_command =          KEY_SLASH;

/* ---- debug & menu -------------------------------------------------------- */
/* TODO: navigate menus with arrow keys */
KeyboardKey bind_left =                     KEY_LEFT;
KeyboardKey bind_right =                    KEY_RIGHT;
KeyboardKey bind_down =                     KEY_DOWN;
KeyboardKey bind_up =                       KEY_UP;

#endif /* MC_C_KEYMAPS_H */

