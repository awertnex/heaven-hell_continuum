#ifndef KEYMAPS_H
#include <raylib.h>

// ---- movement ---------------------------------------------------------------
extern KeyboardKey bind_jump;
extern KeyboardKey bind_sneak;
extern KeyboardKey bind_sprint;
extern KeyboardKey bind_strafe_left;
extern KeyboardKey bind_strafe_right;
extern KeyboardKey bind_walk_backwards;
extern KeyboardKey bind_walk_forwards;

// ---- gameplay ---------------------------------------------------------------
extern MouseButton bind_attack_or_destroy;
extern MouseButton bind_pick_block;
extern MouseButton bind_use_item_or_place_block;

// ---- inventory --------------------------------------------------------------
extern KeyboardKey bind_drop_selected_item;
extern KeyboardKey bind_hotbar_slot_1;
extern KeyboardKey bind_hotbar_slot_2;
extern KeyboardKey bind_hotbar_slot_3;
extern KeyboardKey bind_hotbar_slot_4;
extern KeyboardKey bind_hotbar_slot_5;
extern KeyboardKey bind_hotbar_slot_6;
extern KeyboardKey bind_hotbar_slot_7;
extern KeyboardKey bind_hotbar_slot_8;
extern KeyboardKey bind_hotbar_slot_9;
extern KeyboardKey bind_hotbar_slot_kp_1;
extern KeyboardKey bind_hotbar_slot_kp_2;
extern KeyboardKey bind_hotbar_slot_kp_3;
extern KeyboardKey bind_hotbar_slot_kp_4;
extern KeyboardKey bind_hotbar_slot_kp_5;
extern KeyboardKey bind_hotbar_slot_kp_6;
extern KeyboardKey bind_hotbar_slot_kp_7;
extern KeyboardKey bind_hotbar_slot_kp_8;
extern KeyboardKey bind_hotbar_slot_kp_9;
extern KeyboardKey bind_open_or_close_inventory;
extern KeyboardKey bind_swap_item_with_offhand;

// ---- miscellaneous ----------------------------------------------------------
extern KeyboardKey bind_take_screenshot;
extern KeyboardKey bind_toggle_hud;
extern KeyboardKey bind_toggle_debug;
extern KeyboardKey bind_toggle_cinematic_camera;
extern KeyboardKey bind_toggle_fullscreen;
extern KeyboardKey bind_toggle_perspective;
extern KeyboardKey bind_pause;
extern KeyboardKey bind_chat_or_command;

// ---- debug & menu -----------------------------------------------------------
//TODO: navigate menus with arrow keys
extern KeyboardKey bind_left;
extern KeyboardKey bind_right;
extern KeyboardKey bind_down;
extern KeyboardKey bind_up;
extern KeyboardKey bind_quit;

#define KEYMAPS_H
#endif
