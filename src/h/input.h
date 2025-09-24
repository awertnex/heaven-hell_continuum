#ifndef GAME_KEYMAPS_H
#define GAME_KEYMAPS_H

#include "../engine/h/core.h"
#include "../engine/h/defines.h"

#define KEYS_MAX 120
#define DOUBLE_PRESS_TIME_THRESHOLD 0.4f

enum KeyboardKeyState
{
    KEY_IDLE,
    KEY_PRESS,
    KEY_HOLD,
    KEY_RELEASE,
    KEY_LISTEN_DOUBLE,
    KEY_PRESS_DOUBLE,
    KEY_HOLD_DOUBLE,
    KEY_RELEASE_DOUBLE,
}; /* KeyboardKeyState */

enum KeyboardKeys
{
    KEY_SPACE,
    KEY_APOSTROPHE,
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON,
    KEY_EQUAL,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFT_BRACKET,
    KEY_BACKSLASH,
    KEY_RIGHT_BRACKET,
    KEY_GRAVE_ACCENT,
    KEY_WORLD_1,
    KEY_WORLD_2,

    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CAPS_LOCK,
    KEY_SCROLL_LOCK,
    KEY_NUM_LOCK,
    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_F25,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    KEY_KP_ADD,
    KEY_KP_ENTER,
    KEY_KP_EQUAL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_ALT,
    KEY_LEFT_SUPER,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_CONTROL,
    KEY_RIGHT_ALT,
    KEY_RIGHT_SUPER,
    KEY_MENU,
}; /* KeyboardKeys */

/* ---- section: declarations ----------------------------------------------- */

extern u32 keyboard_key[KEYS_MAX];
extern u32 keyboard_tab[KEYS_MAX];

/* ---- section: movement --------------------------------------------------- */

extern u32 bind_jump;
extern u32 bind_sneak;
extern u32 bind_sprint;
extern u32 bind_strafe_left;
extern u32 bind_strafe_right;
extern u32 bind_walk_backwards;
extern u32 bind_walk_forwards;

/* ---- section: gameplay --------------------------------------------------- */

// extern u32 bind_attack_or_destroy;
// extern u32 bind_pick_block;
// extern u32 bind_use_item_or_place_block;

/* ---- section: inventory -------------------------------------------------- */

// extern u32 bind_drop_selected_item;
// extern u32 bind_hotbar_slot[10];
// extern u32 bind_hotbar_slot_kp[10];
// extern u32 bind_open_or_close_inventory;
// extern u32 bind_swap_item_with_offhand;

/* ---- section: miscellaneous ---------------------------------------------- */

// extern u32 bind_take_screenshot;
extern u32 bind_toggle_hud;
extern u32 bind_toggle_debug;
// extern u32 bind_toggle_cinematic_camera;
// extern u32 bind_toggle_fullscreen;
// extern u32 bind_toggle_perspective;
// extern u32 bind_pause;
// extern u32 bind_chat_or_command;

/* ---- section: debug & menu ----------------------------------------------- */

/* TODO: navigate menus with arrow keys */
// extern u32 bind_left;
// extern u32 bind_right;
// extern u32 bind_down;
// extern u32 bind_up;

/* ---- section: signatures ------------------------------------------------- */

void update_keys(Render *window);

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_listen_double(const u32 key)
{return (keyboard_key[key] == KEY_LISTEN_DOUBLE);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_hold(const u32 key)
{return (keyboard_key[key] == KEY_HOLD);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_hold_double(const u32 key)
{return (keyboard_key[key] == KEY_HOLD_DOUBLE);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_release(const u32 key)
{return (keyboard_key[key] == KEY_RELEASE);}

/*
 * -- INTERNAL USE ONLY --;
 */
static inline b8 _is_key_release_double(const u32 key)
{return (keyboard_key[key] == KEY_RELEASE_DOUBLE);}

static inline b8 is_key_press(const u32 key)
{return (keyboard_key[key] == KEY_PRESS);}

static inline b8 is_key_press_double(const u32 key)
{return (keyboard_key[key] == KEY_PRESS_DOUBLE);}

static inline b8 is_key_hold(const u32 key)
{return (keyboard_key[key] == KEY_HOLD || keyboard_key[key] == KEY_HOLD_DOUBLE);}

static inline b8 is_key_release(const u32 key)
{return (keyboard_key[key] == KEY_RELEASE || keyboard_key[key] == KEY_RELEASE_DOUBLE);}

#endif /* GAME_INPUT_H */

