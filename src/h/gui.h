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
#define COL_TEXTURE_DEFAULT	color(0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
#define COL_TEXT_DEFAULT	color(0xFF, 0xFF, 0xFF, 0xE6, 0xFF)
#define COL_TEXT_HOVER		color(0xE8, 0xE6, 0x91, 0xE6, 0xFF)
#define COL_TRANS_MENU		color(0xFF, 0xFF, 0xFF, 0xFF, 0xBE)
#define COL_SKYBOX			color(0xA4, 0xE6, 0xFF, 0xE6, 0xFF)
#define COL_STATS_0			color(0x4B, 0xC8, 0x64, 0xFF, 0xFF)
#define COL_STATS_1			color(0x64, 0xC8, 0x4B, 0xFF, 0xFF)
#define COL_STATS_2			color(0xC8, 0x64, 0x4B, 0xFF, 0xFF)
#define COL_STATS_3			color(0xC8, 0x4B, 0x5F, 0xFF, 0xFF)
#define COL_X				color(0xFF, 0x32, 0x32, 0xFF, 0xFF)
#define COL_Y				color(0x32, 0xFF, 0x32, 0xFF, 0xFF)
#define COL_Z				color(0x32, 0x32, 0xFF, 0xFF, 0xFF)
#define TINT_BUTTON_HOVER	color(0xC8, 0xD2, 0xFF, 0xFF, 0xFF)

enum ButtonStates
{
	BUTTON_INACTIVE	 = 0,
	BUTTON_LISTENING = 1,
	BUTTON_PRESSED	 = 2,
}; /* ButtonStates */

// ---- declarations -----------------------------------------------------------
extern Vector2 cursor;
#define detect_cursor cursor = GetMousePosition();

extern Font font_regular;
extern Font font_bold;
extern Font font_italic;
extern Font font_bold_italic;
extern u8 font_size;
extern u8 text_row_height;

extern Texture2D	texture_hud_widgets;
extern Texture2D	texture_container_inventory;

extern Rectangle	hud_hotbar;
extern Rectangle	hud_hotbar_selected;
extern Rectangle	hud_hotbar_offhand;
extern Rectangle	hud_crosshair;
extern Rectangle	button_inactive;
extern Rectangle	button;
extern Rectangle	button_selected;
extern Rectangle	container_inventory_survival;
extern Rectangle	container_slot_size;

extern v2i16		hud_hotbar_position;
extern f32			hud_hotbar_slot_selected;
extern v2i16		hud_crosshair_position;
extern u16			game_menu_position;
extern u8			button_spacing_verical;
extern v2i16		container_inventory_survival_position;
extern v2i16		container_inventory_survival_first_slot_position;

extern u8			*container_inventory_slots[5][9];
extern u8			*container_inventory_slots_crafting[5];
extern u8			*container_inventory_slots_armor[5];
extern u8			container_inventory_slots_offhand;
extern u8			container_cursor_slot[2];
extern u8			*hotbar_slots[9][9];

// ---- game menu --------------------------------------------------------------
extern u8			button_state_back_to_game;
extern u8			button_state_options;
extern u8			button_state_save_and_quit_to_title;

// ---- debug info -------------------------------------------------------------
extern str			str_fps[16];
extern str			str_player_position[32];
extern str			str_player_block[32];
extern str			str_player_chunk[32];
extern str			str_player_direction[32];
extern str			str_block_count[32];
extern str			str_quad_count[32];
extern str			str_tri_count[32];
extern str			str_vertex_count[32];
extern u8			font_size_debug_info;
extern Camera3D		camera_debug_info;

// ---- signatures -------------------------------------------------------------
void init_gui();
void free_gui();
void draw_text(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, Color tint);
void draw_text_centered(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, Color tint, u8 center_vertically);
float get_str_width(Font font, const str *str, f32 font_size, f32 spacing);
void draw_texture(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint); /* scale is based on source.scale*scl */
void draw_texture_simple(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint); /* scale is based on scl */
void draw_button(Texture2D texture, Rectangle button, v2i16 pos, u8 *button_state, const str *str);
void draw_game_menu();
void draw_hud();
void draw_inventory_survival();
void draw_debug_info();

#define GUI_H
#endif
