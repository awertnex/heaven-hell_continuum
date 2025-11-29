#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include <engine/h/core.h>
#include <engine/h/defines.h>
#include "main.h"

/* ---- movement ------------------------------------------------------------ */

extern u32 bind_jump;
extern u32 bind_sneak;
extern u32 bind_sprint;
extern u32 bind_strafe_left;
extern u32 bind_strafe_right;
extern u32 bind_walk_backwards;
extern u32 bind_walk_forwards;

/* ---- gameplay ------------------------------------------------------------ */

extern u32 bind_attack_or_destroy;
extern u32 bind_pick_block;
extern u32 bind_use_item_or_place_block;

/* ---- inventory ----------------------------------------------------------- */

/* extern u32 bind_drop_selected_item; */
extern u32 bind_hotbar_slot[SET_HOTBAR_SLOTS_MAX];
extern u32 bind_hotbar_slot_kp[SET_HOTBAR_SLOTS_MAX];
extern u32 bind_inventory;
/* extern u32 bind_swap_item_with_offhand; */

/* ---- misc ---------------------------------------------------------------- */

/* extern u32 bind_take_screenshot; */
extern u32 bind_toggle_hud;
extern u32 bind_toggle_debug;
extern u32 bind_toggle_super_debug;
/* extern u32 bind_toggle_cinematic_camera; */
/* extern u32 bind_toggle_fullscreen; */
extern u32 bind_toggle_perspective;
extern u32 bind_toggle_zoom;
/* extern u32 bind_pause; */
/* extern u32 bind_chat_or_command; */

/* ---- debug --------------------------------------------------------------- */

/* TODO: navigate menus with arrow keys */
/* extern u32 bind_left; */
/* extern u32 bind_right; */
/* extern u32 bind_down; */
/* extern u32 bind_up; */
extern u32 bind_debug_mod;
extern u32 bind_toggle_chunk_queue_visualizer;

#endif /* GAME_INPUT_H */
