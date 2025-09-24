#include "engine/h/core.h"
#include "engine/h/defines.h"
#include "h/input.h"
#include "h/main.h"
#include "h/logic.h"

/* ---- section: declarations ----------------------------------------------- */

u32 keyboard_key[KEYS_MAX];
u32 keyboard_tab[KEYS_MAX] =
{
    GLFW_KEY_SPACE,
    GLFW_KEY_APOSTROPHE,
    GLFW_KEY_COMMA,
    GLFW_KEY_MINUS,
    GLFW_KEY_PERIOD,
    GLFW_KEY_SLASH,
    GLFW_KEY_0,
    GLFW_KEY_1,
    GLFW_KEY_2,
    GLFW_KEY_3,
    GLFW_KEY_4,
    GLFW_KEY_5,
    GLFW_KEY_6,
    GLFW_KEY_7,
    GLFW_KEY_8,
    GLFW_KEY_9,
    GLFW_KEY_SEMICOLON,
    GLFW_KEY_EQUAL,
    GLFW_KEY_A,
    GLFW_KEY_B,
    GLFW_KEY_C,
    GLFW_KEY_D,
    GLFW_KEY_E,
    GLFW_KEY_F,
    GLFW_KEY_G,
    GLFW_KEY_H,
    GLFW_KEY_I,
    GLFW_KEY_J,
    GLFW_KEY_K,
    GLFW_KEY_L,
    GLFW_KEY_M,
    GLFW_KEY_N,
    GLFW_KEY_O,
    GLFW_KEY_P,
    GLFW_KEY_Q,
    GLFW_KEY_R,
    GLFW_KEY_S,
    GLFW_KEY_T,
    GLFW_KEY_U,
    GLFW_KEY_V,
    GLFW_KEY_W,
    GLFW_KEY_X,
    GLFW_KEY_Y,
    GLFW_KEY_Z,
    GLFW_KEY_LEFT_BRACKET,
    GLFW_KEY_BACKSLASH,
    GLFW_KEY_RIGHT_BRACKET,
    GLFW_KEY_GRAVE_ACCENT,
    GLFW_KEY_WORLD_1,
    GLFW_KEY_WORLD_2,

    GLFW_KEY_ESCAPE,
    GLFW_KEY_ENTER,
    GLFW_KEY_TAB,
    GLFW_KEY_BACKSPACE,
    GLFW_KEY_INSERT,
    GLFW_KEY_DELETE,
    GLFW_KEY_RIGHT,
    GLFW_KEY_LEFT,
    GLFW_KEY_DOWN,
    GLFW_KEY_UP,
    GLFW_KEY_PAGE_UP,
    GLFW_KEY_PAGE_DOWN,
    GLFW_KEY_HOME,
    GLFW_KEY_END,
    GLFW_KEY_CAPS_LOCK,
    GLFW_KEY_SCROLL_LOCK,
    GLFW_KEY_NUM_LOCK,
    GLFW_KEY_PRINT_SCREEN,
    GLFW_KEY_PAUSE,
    GLFW_KEY_F1,
    GLFW_KEY_F2,
    GLFW_KEY_F3,
    GLFW_KEY_F4,
    GLFW_KEY_F5,
    GLFW_KEY_F6,
    GLFW_KEY_F7,
    GLFW_KEY_F8,
    GLFW_KEY_F9,
    GLFW_KEY_F10,
    GLFW_KEY_F11,
    GLFW_KEY_F12,
    GLFW_KEY_F13,
    GLFW_KEY_F14,
    GLFW_KEY_F15,
    GLFW_KEY_F16,
    GLFW_KEY_F17,
    GLFW_KEY_F18,
    GLFW_KEY_F19,
    GLFW_KEY_F20,
    GLFW_KEY_F21,
    GLFW_KEY_F22,
    GLFW_KEY_F23,
    GLFW_KEY_F24,
    GLFW_KEY_F25,
    GLFW_KEY_KP_0,
    GLFW_KEY_KP_1,
    GLFW_KEY_KP_2,
    GLFW_KEY_KP_3,
    GLFW_KEY_KP_4,
    GLFW_KEY_KP_5,
    GLFW_KEY_KP_6,
    GLFW_KEY_KP_7,
    GLFW_KEY_KP_8,
    GLFW_KEY_KP_9,
    GLFW_KEY_KP_DECIMAL,
    GLFW_KEY_KP_DIVIDE,
    GLFW_KEY_KP_MULTIPLY,
    GLFW_KEY_KP_SUBTRACT,
    GLFW_KEY_KP_ADD,
    GLFW_KEY_KP_ENTER,
    GLFW_KEY_KP_EQUAL,
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_LEFT_ALT,
    GLFW_KEY_LEFT_SUPER,
    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_RIGHT_ALT,
    GLFW_KEY_RIGHT_SUPER,
    GLFW_KEY_MENU,
}; /* keyboard_tab */

/* ---- section: movement --------------------------------------------------- */

u32 bind_jump                       = KEY_SPACE;
u32 bind_sneak                      = KEY_LEFT_CONTROL;
u32 bind_sprint                     = KEY_LEFT_SHIFT;
u32 bind_strafe_left                = KEY_A;
u32 bind_strafe_right               = KEY_D;
u32 bind_walk_backwards             = KEY_S;
u32 bind_walk_forwards              = KEY_W;

/* ---- section: gameplay --------------------------------------------------- */

u32 bind_attack_or_destroy          = GLFW_MOUSE_BUTTON_LEFT;
u32 bind_pick_block                 = GLFW_MOUSE_BUTTON_MIDDLE;
u32 bind_use_item_or_place_block    = GLFW_MOUSE_BUTTON_RIGHT;

/* ---- section: inventory -------------------------------------------------- */

u32 bind_drop_selected_item         = KEY_Q;
u32 bind_hotbar_slot_1              = KEY_1;
u32 bind_hotbar_slot_2              = KEY_2;
u32 bind_hotbar_slot_3              = KEY_3;
u32 bind_hotbar_slot_4              = KEY_4;
u32 bind_hotbar_slot_5              = KEY_5;
u32 bind_hotbar_slot_6              = KEY_6;
u32 bind_hotbar_slot_7              = KEY_7;
u32 bind_hotbar_slot_8              = KEY_8;
u32 bind_hotbar_slot_9              = KEY_9;
u32 bind_hotbar_slot_0              = KEY_0;
u32 bind_hotbar_slot_kp_1           = KEY_KP_1;
u32 bind_hotbar_slot_kp_2           = KEY_KP_2;
u32 bind_hotbar_slot_kp_3           = KEY_KP_3;
u32 bind_hotbar_slot_kp_4           = KEY_KP_4;
u32 bind_hotbar_slot_kp_5           = KEY_KP_5;
u32 bind_hotbar_slot_kp_6           = KEY_KP_6;
u32 bind_hotbar_slot_kp_7           = KEY_KP_7;
u32 bind_hotbar_slot_kp_8           = KEY_KP_8;
u32 bind_hotbar_slot_kp_9           = KEY_KP_9;
u32 bind_hotbar_slot_kp_0           = KEY_KP_0;
u32 bind_open_or_close_inventory    = KEY_E;
u32 bind_swap_item_with_offhand     = KEY_F;

/* ---- section: miscellaneous ---------------------------------------------- */

u32 bind_take_screenshot            = KEY_F2;
u32 bind_toggle_hud                 = KEY_F1;
u32 bind_toggle_debug               = KEY_F3;
u32 bind_toggle_cinematic_camera    = KEY_F6;
u32 bind_toggle_fullscreen          = KEY_F11;
u32 bind_toggle_perspective         = KEY_F5;
u32 bind_pause                      = KEY_ESCAPE;
u32 bind_chat_or_command            = KEY_SLASH;

/* ---- section: debug & menu ----------------------------------------------- */

/* TODO: navigate menus with arrow keys */
u32 bind_left                       = KEY_LEFT;
u32 bind_right                      = KEY_RIGHT;
u32 bind_down                       = KEY_DOWN;
u32 bind_up                         = KEY_UP;

void update_keys(Render *render)
{
    static f64 key_press_start_time[KEYS_MAX];
    for (u32 i = 0; i < KEYS_MAX; ++i)
    {
        b8 key_press = (glfwGetKey(render->window, keyboard_tab[i]) == GLFW_PRESS);
        b8 key_release = (glfwGetKey(render->window, keyboard_tab[i]) == GLFW_RELEASE);

        if (key_press && !keyboard_key[i])
        {
            keyboard_key[i] = KEY_PRESS;
            key_press_start_time[i] = render->frame_start;
            continue;
        }
        else if (key_press && _is_key_listen_double(i))
        {
            if (glfwGetTime() - key_press_start_time[i] < DOUBLE_PRESS_TIME_THRESHOLD)
                keyboard_key[i] = KEY_PRESS_DOUBLE;
            else
                keyboard_key[i] = KEY_PRESS;
            key_press_start_time[i] = render->frame_start;
            continue;
        }

        if (key_release)
        {
            if (_is_key_hold(i))
                keyboard_key[i] = KEY_RELEASE;
            else if (_is_key_hold_double(i))
                keyboard_key[i] = KEY_RELEASE_DOUBLE;
            continue;
        }

        if (is_key_press(i))            keyboard_key[i] = KEY_HOLD;
        if (is_key_press_double(i))     keyboard_key[i] = KEY_HOLD_DOUBLE;
        if (_is_key_release(i))         keyboard_key[i] = KEY_LISTEN_DOUBLE;
        if (_is_key_release_double(i))  keyboard_key[i] = KEY_IDLE;
    }
}

