#ifndef GUI_H
#include <raylib.h>

#define VECTOR2_TYPES
#define VECTOR3_TYPES
#include <defines.h>

#define show_cursor ShowCursor()
#define hide_cursor HideCursor()
#define center_cursor SetMousePosition(WIDTH/2, HEIGHT/2)
#define color(r, g, b, v, a) (Color){((f32)r/255)*v, ((f32)g/255)*v, ((f32)b/255)*v, a}
#define draw_menu_overlay DrawRectangle(0, 0, win.scl.x, win.scl.y, COL_MENU_BG_OVERLAY)

// ---- colors -----------------------------------------------------------------
#define COL_MENU_BG_OVERLAY color(0x00, 0x00, 0x00, 0xFF, 0x46)
#define COL_TEXTURE_DEFAULT color(0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
#define COL_TEXT_DEFAULT    color(0xFF, 0xFF, 0xFF, 0xE6, 0xFF)
#define COL_TEXT_HOVER      color(0xE8, 0xE6, 0x91, 0xE6, 0xFF)
#define COL_TRANS_MENU      color(0xFF, 0xFF, 0xFF, 0xFF, 0xBE)
#define COL_SKYBOX          color(0xA4, 0xE6, 0xFF, 0xE6, 0xFF)
#define COL_STATS_0         color(0x4B, 0xC8, 0x64, 0xFF, 0xFF)
#define COL_STATS_1         color(0x64, 0xC8, 0x4B, 0xFF, 0xFF)
#define COL_STATS_2         color(0xC8, 0x64, 0x4B, 0xFF, 0xFF)
#define COL_STATS_3         color(0xC8, 0x4B, 0x5F, 0xFF, 0xFF)
#define COL_X               color(0xFF, 0x32, 0x32, 0xFF, 0xFF)
#define COL_Y               color(0x32, 0xFF, 0x32, 0xFF, 0xFF)
#define COL_Z               color(0x32, 0x32, 0xFF, 0xFF, 0xFF)
#define TINT_BUTTON_HOVER   color(0xC8, 0xD2, 0xFF, 0xFF, 0xFF)

// ---- declarations -----------------------------------------------------------
extern Vector2 cursor;
#define detect_cursor cursor = GetMousePosition();

extern Font font_regular;
extern Font font_bold;
extern Font font_italic;
extern Font font_bold_italic;
extern u8 font_size;
extern u8 text_row_height;

extern Texture2D texture_hud_widgets;
extern Texture2D texture_container_inventory;

extern Rectangle hud_hotbar;
extern Rectangle hud_hotbar_selected;
extern Rectangle hud_hotbar_offhand;
extern Rectangle hud_crosshair;
extern Rectangle button_inactive;
extern Rectangle button;
extern Rectangle button_selected;
extern Rectangle container_inventory;
extern Rectangle container_slot_size;

extern v2i16 hud_hotbar_position;
extern f32 hud_hotbar_slot_selected;
extern v2i16 hud_crosshair_position;
extern u16 game_menu_position;
extern u8 button_spacing_verical;
extern v2i16 container_inventory_position;
extern v2i16 container_inventory_first_slot_position;

extern u8 *container_inventory_slots[5][9];
extern u8 *container_inventory_slots_crafting[5];
extern u8 *container_inventory_slots_armor[5];
extern u8 container_inventory_slots_offhand;
extern u8 container_cursor_slot[2];
extern u8 *hotbar_slots[9][9];

extern u16 menu_index;
enum MenuNames
{
    MENU_TITLE = 1,
    MENU_SINGLEPLAYER = 2,
    MENU_MULTIPLAYER = 3,
    MENU_MINECRAFT_C_REALMS = 4,
    MENU_OPTIONS = 5,
    MENU_GAME = 6,

}; /* MenuNames */

// ---- button stuff -----------------------------------------------------------
enum ButtonStates
{
    BTN_COUNT = 111,
    BTN_INACTIVE = 0,
    BTN_ACTIVE = 1,
    BTN_PRESSED = 2,
}; /* ButtonStates */

extern u8 buttons[BTN_COUNT];
enum ButtonNames
{
    // ---- title screen -------------------------------------------------------
    BTN_SINGLEPLAYER = 1,
    BTN_MULTIPLAYER = 2,
    BTN_MINECRAFT_C_REALMS = 3,
    BTN_OPTIONS = 4,
    BTN_QUIT = 5,

    // ---- game menu ----------------------------------------------------------
    BTN_BACK_TO_GAME = 0,
    BTN_ADVANCEMENTS = 2,
    BTN_STATISTICS = 3,
    BTN_GIVE_FEEDBACK = 6,
    BTN_REPORT_BUGS = 7,
    BTN_OPEN_TO_LAN = 8,

    // ---- inventory ----------------------------------------------------------
    BTN_INV_1 = 1,
    BTN_INV_2 = 2,
    BTN_INV_3 = 3,
    BTN_INV_4 = 4,
    BTN_INV_5 = 5,
    BTN_INV_6 = 6,
    BTN_INV_7 = 7,
    BTN_INV_8 = 8,
    BTN_INV_9 = 9,
    BTN_INV_10 = 10,
    BTN_INV_11 = 11,
    BTN_INV_12 = 12,
    BTN_INV_13 = 13,
    BTN_INV_14 = 14,
    BTN_INV_15 = 15,
    BTN_INV_16 = 16,
    BTN_INV_17 = 17,
    BTN_INV_18 = 18,
    BTN_INV_19 = 19,
    BTN_INV_20 = 20,
    BTN_INV_21 = 21,
    BTN_INV_22 = 22,
    BTN_INV_23 = 23,
    BTN_INV_24 = 24,
    BTN_INV_25 = 25,
    BTN_INV_26 = 26,
    BTN_INV_27 = 27,
    BTN_ARMOR_1 = 28,
    BTN_ARMOR_2 = 29,
    BTN_ARMOR_3 = 30,
    BTN_ARMOR_4 = 31,
    BTN_SHIELD = 32,
    BTN_OFFHAND = 33,

    // ---- hotbar slots -------------------------------------------------------
    BTN_HOTBAR_1 = 34,
    BTN_HOTBAR_2 = 35,
    BTN_HOTBAR_3 = 36,
    BTN_HOTBAR_4 = 37,
    BTN_HOTBAR_5 = 38,
    BTN_HOTBAR_6 = 39,
    BTN_HOTBAR_7 = 40,
    BTN_HOTBAR_8 = 41,
    BTN_HOTBAR_9 = 42,

    // ---- functional ---------------------------------------------------------
    BTN_ITEM_IN_1 = 43,
    BTN_ITEM_IN_2 = 44,
    BTN_ITEM_IN_3 = 45,
    BTN_ITEM_IN_4 = 46,
    BTN_ITEM_IN_5 = 47,
    BTN_ITEM_IN_6 = 48,
    BTN_ITEM_IN_7 = 49,
    BTN_ITEM_IN_8 = 50,
    BTN_ITEM_IN_9 = 51,
    BTN_ITEM_OUT_1 = 52,
    BTN_ITEM_OUT_2 = 53,
    BTN_ITEM_OUT_3 = 54,

    // ---- containers ---------------------------------------------------------
    BTN_CONTAINER_1 = 55,
    BTN_CONTAINER_2 = 56,
    BTN_CONTAINER_3 = 57,
    BTN_CONTAINER_4 = 58,
    BTN_CONTAINER_5 = 59,
    BTN_CONTAINER_6 = 60,
    BTN_CONTAINER_7 = 61,
    BTN_CONTAINER_8 = 62,
    BTN_CONTAINER_9 = 63,
    BTN_CONTAINER_10 = 64,
    BTN_CONTAINER_11 = 65,
    BTN_CONTAINER_12 = 66,
    BTN_CONTAINER_13 = 67,
    BTN_CONTAINER_14 = 68,
    BTN_CONTAINER_15 = 69,
    BTN_CONTAINER_16 = 70,
    BTN_CONTAINER_17 = 71,
    BTN_CONTAINER_18 = 72,
    BTN_CONTAINER_19 = 73,
    BTN_CONTAINER_20 = 74,
    BTN_CONTAINER_21 = 75,
    BTN_CONTAINER_22 = 76,
    BTN_CONTAINER_23 = 77,
    BTN_CONTAINER_24 = 78,
    BTN_CONTAINER_25 = 79,
    BTN_CONTAINER_26 = 80,
    BTN_CONTAINER_27 = 81,
    BTN_CONTAINER_28 = 82,
    BTN_CONTAINER_29 = 83,
    BTN_CONTAINER_30 = 84,
    BTN_CONTAINER_31 = 85,
    BTN_CONTAINER_32 = 86,
    BTN_CONTAINER_33 = 87,
    BTN_CONTAINER_34 = 88,
    BTN_CONTAINER_35 = 89,
    BTN_CONTAINER_36 = 90,
    BTN_CONTAINER_37 = 91,
    BTN_CONTAINER_38 = 92,
    BTN_CONTAINER_39 = 93,
    BTN_CONTAINER_40 = 94,
    BTN_CONTAINER_41 = 95,
    BTN_CONTAINER_42 = 96,
    BTN_CONTAINER_43 = 97,
    BTN_CONTAINER_44 = 98,
    BTN_CONTAINER_45 = 99,
    BTN_CONTAINER_46 = 100,
    BTN_CONTAINER_47 = 101,
    BTN_CONTAINER_48 = 102,
    BTN_CONTAINER_49 = 103,
    BTN_CONTAINER_50 = 104,
    BTN_CONTAINER_51 = 105,
    BTN_CONTAINER_52 = 106,
    BTN_CONTAINER_53 = 107,
    BTN_CONTAINER_54 = 108,

    // ---- super debugger (SDB) -----------------------------------------------
    BTN_SDB_ADD = 109,
    BTN_SDB_SUB = 110,
}; /* ButtonNames */

// ---- debug info -------------------------------------------------------------
extern str str_fps[16];
extern str str_player_position[32];
extern str str_player_block[32];
extern str str_player_chunk[32];
extern str str_player_direction[32];
extern str str_block_count[32];
extern str str_quad_count[32];
extern str str_tri_count[32];
extern str str_vertex_count[32];
extern u8 font_size_debug_info;
extern Camera3D camera_debug_info;

// ---- signatures -------------------------------------------------------------
void init_gui();
void apply_render_settings();
void free_gui();

void update_menus(u16 *player_state);
void draw_hud();
void draw_inventory();
void draw_debug_info();

void draw_text(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, Color tint);
void draw_text_centered(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, Color tint, u8 center_vertically);
float get_str_width(Font font, const str *str, f32 font_size, f32 spacing);
void draw_texture(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint); /* scale is based on source.scale*scl */
void draw_texture_simple(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint); /* scale is based on scl */
void draw_button(Texture2D texture, Rectangle button, v2i16 pos, u8 btn_state, void (*func)(), const str *str);

void btn_func_singleplayer();
void btn_func_multiplayer();
void btn_func_minecraft_c_realms();
void btn_func_options();
void btn_func_quit();

void btn_func_back_to_game();
void btn_func_save_and_quit_to_title();

#define GUI_H
#endif
