#ifndef KEYMAPS_H
#include <raylib.h>

enum KeyBinds
{
// ---- movement ---------------------------------------------------------------
BIND_JUMP =                     KEY_SPACE,
BIND_SNEAK =                    KEY_LEFT_CONTROL,
BIND_SPRINT =                   KEY_LEFT_SHIFT,
BIND_STRAFE_LEFT =              KEY_A,
BIND_STRAFE_RIGHT =             KEY_D,
BIND_WALK_BACKWARDS =           KEY_S,
BIND_WALK_FORWARDS =            KEY_W,

// ---- gameplay ---------------------------------------------------------------
BIND_ATTACK_OR_DESTROY =        MOUSE_BUTTON_LEFT,
BIND_PICK_BLOCK =               MOUSE_BUTTON_MIDDLE,
BIND_USE_ITEM_OR_PLACE_BLOCK =  MOUSE_BUTTON_RIGHT,

// ---- inventory --------------------------------------------------------------
BIND_DROP_SELECTED_ITEM =       KEY_Q,
BIND_HOTBAR_SLOT_1 =            KEY_ONE,
BIND_HOTBAR_SLOT_2 =            KEY_TWO,
BIND_HOTBAR_SLOT_3 =            KEY_THREE,
BIND_HOTBAR_SLOT_4 =            KEY_FOUR,
BIND_HOTBAR_SLOT_5 =            KEY_FIVE,
BIND_HOTBAR_SLOT_6 =            KEY_SIX,
BIND_HOTBAR_SLOT_7 =            KEY_SEVEN,
BIND_HOTBAR_SLOT_8 =            KEY_EIGHT,
BIND_HOTBAR_SLOT_9 =            KEY_NINE,
BIND_HOTBAR_SLOT_KP_1 =         KEY_KP_1,
BIND_HOTBAR_SLOT_KP_2 =         KEY_KP_2,
BIND_HOTBAR_SLOT_KP_3 =         KEY_KP_3,
BIND_HOTBAR_SLOT_KP_4 =         KEY_KP_4,
BIND_HOTBAR_SLOT_KP_5 =         KEY_KP_5,
BIND_HOTBAR_SLOT_KP_6 =         KEY_KP_6,
BIND_HOTBAR_SLOT_KP_7 =         KEY_KP_7,
BIND_HOTBAR_SLOT_KP_8 =         KEY_KP_8,
BIND_HOTBAR_SLOT_KP_9 =         KEY_KP_9,
BIND_OPEN_OR_CLOSE_INVENTORY =  KEY_E,
BIND_SWAP_ITEM_WITH_OFFHAND =   KEY_F,

// ---- miscellaneous ----------------------------------------------------------
BIND_TAKE_SCREENSHOT =          KEY_F2,
BIND_TOGGLE_HUD =               KEY_F1,
BIND_TOGGLE_DEBUG =             KEY_F3,
BIND_TOGGLE_CINEMATIC_CAMERA =  KEY_F6,
BIND_TOGGLE_FULLSCREEN =        KEY_F11,
BIND_TOGGLE_PERSPECTIVE =       KEY_F5,
BIND_PAUSE =                    KEY_ESCAPE,
BIND_CHAT_OR_COMMAND =          KEY_SLASH,

// ---- debug & menu -----------------------------------------------------------
//TODO: navigate menus with arrow keys
BIND_LEFT =                     KEY_LEFT,
BIND_RIGHT =                    KEY_RIGHT,
BIND_DOWN =                     KEY_DOWN,
BIND_UP =                       KEY_UP,
BIND_QUIT =                     KEY_Q,
}; /* KeyBinds */

#define KEYMAPS_H
#endif
