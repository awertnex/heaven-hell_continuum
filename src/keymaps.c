#include <raylib.h>
#include "h/keymaps.h"

// ---- movement ---------------------------------------------------------------
KeyboardKey BIND_JUMP =                     KEY_SPACE;
KeyboardKey BIND_SNEAK =                    KEY_LEFT_CONTROL;
KeyboardKey BIND_SPRINT =                   KEY_LEFT_SHIFT;
KeyboardKey BIND_STRAFE_LEFT =              KEY_A;
KeyboardKey BIND_STRAFE_RIGHT =             KEY_D;
KeyboardKey BIND_WALK_BACKWARDS =           KEY_S;
KeyboardKey BIND_WALK_FORWARDS =            KEY_W;

// ---- gameplay ---------------------------------------------------------------
MouseButton BIND_ATTACK_OR_DESTROY =        MOUSE_BUTTON_LEFT;
MouseButton BIND_PICK_BLOCK =               MOUSE_BUTTON_MIDDLE;
MouseButton BIND_USE_ITEM_OR_PLACE_BLOCK =  MOUSE_BUTTON_RIGHT;

// ---- inventory --------------------------------------------------------------
KeyboardKey BIND_DROP_SELECTED_ITEM =       KEY_Q;
KeyboardKey BIND_HOTBAR_SLOT_1 =            KEY_ONE;
KeyboardKey BIND_HOTBAR_SLOT_2 =            KEY_TWO;
KeyboardKey BIND_HOTBAR_SLOT_3 =            KEY_THREE;
KeyboardKey BIND_HOTBAR_SLOT_4 =            KEY_FOUR;
KeyboardKey BIND_HOTBAR_SLOT_5 =            KEY_FIVE;
KeyboardKey BIND_HOTBAR_SLOT_6 =            KEY_SIX;
KeyboardKey BIND_HOTBAR_SLOT_7 =            KEY_SEVEN;
KeyboardKey BIND_HOTBAR_SLOT_8 =            KEY_EIGHT;
KeyboardKey BIND_HOTBAR_SLOT_9 =            KEY_NINE;
KeyboardKey BIND_HOTBAR_SLOT_KP_1 =         KEY_KP_1;
KeyboardKey BIND_HOTBAR_SLOT_KP_2 =         KEY_KP_2;
KeyboardKey BIND_HOTBAR_SLOT_KP_3 =         KEY_KP_3;
KeyboardKey BIND_HOTBAR_SLOT_KP_4 =         KEY_KP_4;
KeyboardKey BIND_HOTBAR_SLOT_KP_5 =         KEY_KP_5;
KeyboardKey BIND_HOTBAR_SLOT_KP_6 =         KEY_KP_6;
KeyboardKey BIND_HOTBAR_SLOT_KP_7 =         KEY_KP_7;
KeyboardKey BIND_HOTBAR_SLOT_KP_8 =         KEY_KP_8;
KeyboardKey BIND_HOTBAR_SLOT_KP_9 =         KEY_KP_9;
KeyboardKey BIND_OPEN_OR_CLOSE_INVENTORY =  KEY_E;
KeyboardKey BIND_SWAP_ITEM_WITH_OFFHAND =   KEY_F;

// ---- miscellaneous ----------------------------------------------------------
KeyboardKey BIND_TAKE_SCREENSHOT =          KEY_F2;
KeyboardKey BIND_TOGGLE_HUD =               KEY_F1;
KeyboardKey BIND_TOGGLE_DEBUG =             KEY_F3;
KeyboardKey BIND_TOGGLE_CINEMATIC_CAMERA =  KEY_F6;
KeyboardKey BIND_TOGGLE_FULLSCREEN =        KEY_F11;
KeyboardKey BIND_TOGGLE_PERSPECTIVE =       KEY_F5;
KeyboardKey BIND_PAUSE =                    KEY_ESCAPE;
KeyboardKey BIND_CHAT_OR_COMMAND =          KEY_SLASH;

// ---- debug & menu -----------------------------------------------------------
//TODO: navigate menus with arrow keys
KeyboardKey BIND_LEFT =                     KEY_LEFT;
KeyboardKey BIND_RIGHT =                    KEY_RIGHT;
KeyboardKey BIND_DOWN =                     KEY_DOWN;
KeyboardKey BIND_UP =                       KEY_UP;
KeyboardKey BIND_QUIT =                     KEY_Q;
