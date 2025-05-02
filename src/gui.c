#include <stdio.h>
#include <string.h>

#include "dependencies/raylib-5.5/src/raylib.h"
#include "dependencies/raylib-5.5/src/rlgl.h"

#include "h/gui.h"
#include "h/chunking.h"
#include "h/logic.h"

// ---- variables --------------------------------------------------------------
Vector2 cursor;
Image mc_c_icon;
Font font_regular;
Font font_bold;
Font font_italic;
Font font_bold_italic;
u8 font_size = 0;
u8 text_row_height = 0;

Texture2D texture_hud_widgets;
Texture2D texture_container_inventory;
Texture2D texture_bg;

Rectangle hotbar =              {0.0f, 0.0f, 202.0f, 22.0f};
Rectangle hotbar_selected =     {0.0f, 22.0f, 24.0f, 24.0f};
Rectangle hotbar_offhand =      {24.0f, 22.0f, 22.0f, 24.0f};
Rectangle crosshair =           {240.0f, 0.0f, 16.0f, 16.0f};
Rectangle button_inactive =     {0.0f, 46.0f, 200.0f, 20.0f};
Rectangle button =              {0.0f, 66.0f, 200.0f, 20.0f};
Rectangle button_selected =     {0.0f, 86.0f, 200.0f, 20.0f};
Rectangle container_inventory = {0.0f, 0.0f, 176.0f, 184.0f};
Rectangle container_slot_size = {0.0f, 0.0f, 16.0f, 16.0f};
Rectangle rect_bg =             {0.0f, 0.0f, 16.0f, 16.0f};
Rectangle rect_bg_dark =        {16.0f, 0.0f, 16.0f, 16.0f};

v2i16 hotbar_pos;
f32 hotbar_slot_selected = 1.0f;
v2i16 crosshair_pos;
u16 game_menu_pos = HEIGHT / 3;
u8 button_spacing_vertical = 5;
v2i16 container_inventory_pos;
v2i16 container_inventory_first_slot_Pos;

// TODO: you know what TODO
u8 *container_inventory_slots[5][9];
u8 *container_inventory_slots_crafting[5];
u8 *container_inventory_slots_armor[5];
u8 container_inventory_slots_offhand;
u8 container_cursor_slot[2];
u8 *hotbar_slots[9][9];

u16 menu_index;
u16 menu_layer[5] = {0};
u8 buttons[BTN_COUNT];
static b8 check_menu_ready;

// ---- debug info -------------------------------------------------------------
str str_fps[16];
str str_player_pos[32];
str str_player_block[32];
str str_player_chunk[32];
str str_player_direction[32];
str str_block_count[32];
str str_quad_count[32];
str str_tri_count[32];
str str_vertex_count[32];
u8 font_size_debug_info = 22;

// ---- functions --------------------------------------------------------------
static void print_menu_layers() {
    static str menu_names[10][24] = {
        "",
        "MENU_TITLE",
        "MENU_SINGLEPLAYER",
        "MENU_MULTIPLAYER",
        "MENU_MINECRAFT_C_REALMS",
        "MENU_OPTIONS",
        "MENU_OPTIONS_GAME",
        "MENU_VIDEO_SETTINGS",
        "MENU_GAME",
        "MENU_DEATH",
    };
    printf("menu layers:\n");
    for (u8 i = 0; i < 9; ++i) {
        printf("layer %1d: %s\n", i, menu_names[menu_layer[i]]);
    }
    putchar('\n');
}

void init_fonts() {
    font_regular =      LoadFont("fonts/minecraft_regular.otf");
    font_bold =         LoadFont("fonts/minecraft_bold.otf");
    font_italic =       LoadFont("fonts/minecraft_italic.otf");
    font_bold_italic =  LoadFont("fonts/minecraft_bold_italic.otf");
}

void init_gui() {
    mc_c_icon = LoadImage("resources/logo/128x128.png");
    SetWindowIcon(mc_c_icon);

    texture_hud_widgets =           LoadTexture("resources/gui/widgets.png");
    texture_container_inventory =   LoadTexture("resources/gui/containers/inventory.png");
    texture_bg =                    LoadTexture("resources/gui/bg_options.png");

    menu_index = MENU_TITLE;
    state_menu_depth = 1;
    memset(buttons, 0, BTN_COUNT);
}

void apply_render_settings() {
    setting = (settings){
            .reach_distance =       SETTING_REACH_DISTANCE_MAX,
            .fov =                  SETTING_FOV_DEFAULT,
            .mouse_sensitivity =    SETTING_MOUSE_SENSITIVITY_DEFAULT / 650.0f,
            .render_distance =      SETTING_RENDER_DISTANCE_DEFAULT,
            .gui_scale =            SETTING_GUI_SCALE_DEFAULT,
        };
    font_size = 14 * setting.gui_scale;
    text_row_height = 8 * setting.gui_scale;
}

void update_render_settings(v2f32 render_size) {
    hotbar_pos =                (v2i16){render_size.x / 2, render_size.y - (2 * setting.gui_scale)};
    crosshair_pos =             (v2i16){render_size.x / 2, render_size.y / 2};
    container_inventory_pos =    (v2i16){render_size.x / 2, render_size.y / 2};
}

void free_gui() {
    UnloadImage(mc_c_icon);
    UnloadFont(font_regular);
    UnloadFont(font_bold);
    UnloadFont(font_italic);
    UnloadFont(font_bold_italic);
    UnloadTexture(texture_hud_widgets);
    UnloadTexture(texture_container_inventory);
    UnloadTexture(texture_bg);
}

//jump
void draw_texture_a(Texture2D texture, Rectangle source, Rectangle dest, v2i16 pos, v2i16 scl, Color tint) {
    // scale is based on (source.scale * scl)

    if ((texture.id <= 0) || (scl.x <= 0.0f) || (scl.y <= 0.0f)) return;
    if ((source.width == 0.0f) || (source.height == 0.0f)) return;
    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0.0f, 0.0f, 1.0f);

    i32 tile_width = source.width * scl.x;
    i32 tile_height = source.height * scl.y;

    // top left
    rlTexCoord2f(source.x / texture.width, source.y / texture.height);
    rlVertex2f(pos.x, pos.y);

    // bottom left
    rlTexCoord2f(source.x / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x, pos.y + tile_height);

    // bottom right
    rlTexCoord2f((source.x + source.width) / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y + tile_height);

    // top right
    rlTexCoord2f((source.x + source.width) / texture.width, source.y / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y);
}

void update_menus(v2f32 render_size) {
    if (!menu_index)
        return;

    detect_cursor;
    switch (menu_index) {
        case MENU_TITLE:
            if (!check_menu_ready) {
                menu_layer[state_menu_depth] = MENU_TITLE;
                menu_index = MENU_TITLE;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_SINGLEPLAYER] = 1;
                buttons[BTN_MULTIPLAYER] = 1;
                buttons[BTN_MINECRAFT_C_REALMS] = 1;
                buttons[BTN_OPTIONS] = 1;
                buttons[BTN_QUIT] = 1;
                check_menu_ready = 1;
            }

            //jump
            draw_texture_a(texture_bg, rect_bg, (Rectangle){0.0f, 0.0f, 16.0f, 16.0f},
                    (v2i16){0, 0}, (v2i16){4, 4},
                    COL_TEXTURE_DEFAULT);
            draw_texture_a(texture_bg, rect_bg, (Rectangle){0.0f, 0.0f, 24.0f, 16.0f},
                    (v2i16){0, 64}, (v2i16){4, 4},
                    COL_TEXTURE_DEFAULT);
            draw_texture_a(texture_bg, rect_bg, (Rectangle){0.0f, 0.0f, 32.0f, 32.0f},
                    (v2i16){0, 128}, (v2i16){4, 4},
                    COL_TEXTURE_DEFAULT);

            draw_text(font_regular, MC_C_VERSION,
                    (v2i16){6, render_size.y - 3},
                    font_size, 2, 0, 2, COL_TEXT_DEFAULT);

            draw_text(font_regular, MC_C_AUTHOR,
                    (v2i16){
                    render_size.x - 2,
                    render_size.y - 3},
                    font_size, 2, 2, 2, COL_TEXT_DEFAULT);

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_SINGLEPLAYER,
                    &btn_func_singleplayer,
                    "Singleplayer");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + ((button.height + button_spacing_vertical) * setting.gui_scale)},
                    1, 1,
                    BTN_MULTIPLAYER,
                    &btn_func_multiplayer,
                    "Multiplayer");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 2) * setting.gui_scale)},
                    1, 1,
                    BTN_MINECRAFT_C_REALMS,
                    &btn_func_minecraft_c_realms,
                    "Minecraft.c Realms");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 3) * setting.gui_scale)},
                    1, 1,
                    BTN_OPTIONS,
                    &btn_func_options,
                    "Options...");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 4) * setting.gui_scale)},
                    1, 1,
                    BTN_QUIT,
                    &btn_func_quit,
                    "Quit Game");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_OPTIONS:
            if (!check_menu_ready) {
                menu_layer[state_menu_depth] = MENU_OPTIONS;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_DONE] = 1;
                buttons[BTN_FOV] = 1;
                buttons[BTN_ONLINE] = 1;
                buttons[BTN_SKIN_CUSTOMIZATION] = 1;
                buttons[BTN_MUSIC_N_SOUNDS] = 1;
                buttons[BTN_VIDEO_SETTINGS] = 1;
                buttons[BTN_CONTROLS] = 1;
                buttons[BTN_LANGUAGE] = 1;
                buttons[BTN_CHAT_SETTINGS] = 1;
                buttons[BTN_RESOURCE_PACKS] = 1;
                buttons[BTN_ACCESSIBILITY_SETTINGS] = 1;
                buttons[BTN_TELEMETRY_DATA] = 1;
                buttons[BTN_CREDITS_N_ATTRIBUTION] = 1;
                check_menu_ready = 1;
            }

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_OPTIONS_GAME:
            if (!check_menu_ready) {
                menu_layer[state_menu_depth] = MENU_OPTIONS_GAME;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_DONE] = 1;
                buttons[BTN_FOV] = 1;
                buttons[BTN_DIFFICULTY] = 1;
                buttons[BTN_SKIN_CUSTOMIZATION] = 1;
                buttons[BTN_MUSIC_N_SOUNDS] = 1;
                buttons[BTN_VIDEO_SETTINGS] = 1;
                buttons[BTN_CONTROLS] = 1;
                buttons[BTN_LANGUAGE] = 1;
                buttons[BTN_CHAT_SETTINGS] = 1;
                buttons[BTN_RESOURCE_PACKS] = 1;
                buttons[BTN_ACCESSIBILITY_SETTINGS] = 1;
                buttons[BTN_TELEMETRY_DATA] = 1;
                buttons[BTN_CREDITS_N_ATTRIBUTION] = 1;
                check_menu_ready = 1;
            }

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;


        case MENU_GAME:
            if (!check_menu_ready) {
                menu_layer[state_menu_depth] = MENU_GAME;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_BACK_TO_GAME] = 1;
                buttons[BTN_ADVANCEMENTS] = 1;
                buttons[BTN_GIVE_FEEDBACK] = 1;
                buttons[BTN_OPTIONS_GAME] = 1;
                buttons[BTN_QUIT] = 1;
                check_menu_ready = 1;
            }

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_BACK_TO_GAME,
                    &btn_func_back_to_game,
                    "Back to Game");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + ((button.height + button_spacing_vertical) * setting.gui_scale)},
                    1, 1,
                    BTN_ADVANCEMENTS,
                    &btn_func_options_game,
                    "Advancements");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 2) * setting.gui_scale)},
                    1, 1,
                    BTN_GIVE_FEEDBACK,
                    &btn_func_options_game,
                    "Give Feedback");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 3) * setting.gui_scale)},
                    1, 1,
                    BTN_OPTIONS_GAME,
                    &btn_func_options_game,
                    "Options...");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 4) * setting.gui_scale)},
                    1, 1,
                    BTN_QUIT,
                    &btn_func_save_and_quit_to_title,
                    "Save and Quit to Title");

            rlEnd();
            rlSetTexture(0);
            break;
    }
}

void draw_hud() {
    rlBegin(RL_QUADS);

    draw_texture(texture_hud_widgets, hotbar,
            hotbar_pos,
            (v2i16){setting.gui_scale, setting.gui_scale},
            1, 2, COL_TEXTURE_DEFAULT);

    draw_texture(texture_hud_widgets, hotbar_selected,
            (v2i16){
            hotbar_pos.x - 2 - ((hotbar.width / 2) * setting.gui_scale) + ((hotbar.height - 2) * setting.gui_scale * (hotbar_slot_selected - 1)),
            hotbar_pos.y + setting.gui_scale}, // TODO: revise gui_scale mod of selected hotbar position Y
            (v2i16){
            setting.gui_scale,
            setting.gui_scale},
            0, 2, COL_TEXTURE_DEFAULT);

    draw_texture(texture_hud_widgets, hotbar_offhand,
            (v2i16){
            hotbar_pos.x - ((hotbar.width / 2) * setting.gui_scale) - (hotbar.height * 2 * setting.gui_scale),
            hotbar_pos.y + setting.gui_scale}, 
            (v2i16){
            setting.gui_scale,
            setting.gui_scale},
            0, 2, COL_TEXTURE_DEFAULT);

    if (!(state & STATE_DEBUG))
        draw_texture(texture_hud_widgets, crosshair,
                crosshair_pos, 
                (v2i16){setting.gui_scale, setting.gui_scale},
                0, 0, COL_TEXTURE_DEFAULT);

    rlEnd();
    rlSetTexture(0);
}

void draw_containers(Player *player, v2f32 render_size) {
    draw_menu_overlay(render_size);
    rlBegin(RL_QUADS);

    switch (player->container_state) {
        case CONTR_ANVIL:
            break;

        case CONTR_BEACON:
            break;

        case CONTR_BLAST_FURNACE:
            break;

        case CONTR_BREWING_STAND:
            break;

        case CONTR_CARTOGRAPHY_TABLE:
            break;

        case CONTR_CHEST:
            break;

        case CONTR_CHEST_LARGE:
            break;

        case CONTR_CRAFTING_TABLE:
            break;

        case CONTR_DISPENSER:
            break;

        case CONTR_ENCHANTING_TABLE:
            break;

        case CONTR_FURNACE:
            break;

        case CONTR_GAMEMODE_SWITCHER:
            break;

        case CONTR_GRINDSTONE:
            break;

        case CONTR_HOPPER:
            break;

        case CONTR_HORSE:
            break;

        case CONTR_INVENTORY:
            draw_texture(texture_container_inventory,
                    container_inventory,
                    container_inventory_pos, 
                    (v2i16){ setting.gui_scale, setting.gui_scale},
                    1, 1, COL_TEXTURE_DEFAULT);
            break;

        case CONTR_LEGACY_SMITHING:
            break;

        case CONTR_LOOM:
            break;

        case CONTR_SMITHING:
            break;

        case CONTR_SMOKER:
            break;

        case CONTR_STONECUTTER:
            break;

        case CONTR_VILLAGER:
            break;

        case CONTR_TAB_INVENTORY:
            break;

        case CONTR_TAB_ITEMS:
            break;

        case CONTR_TAB_ITEMS_SEARCH:
            break;
    }

    rlEnd();
    rlSetTexture(0);
}

void draw_debug_info(Camera3D *camera) {
    if (!(state & STATE_DEBUG)) return;

    update_debug_strings();

    // TODO: rewrite DrawRectangle, get rectangle correct size for font
    DrawRectangle(MARGIN - 2, MARGIN,                       get_str_width(font_regular, str_fps,                font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height,     get_str_width(font_regular, str_player_pos,         font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height * 2, get_str_width(font_regular, str_player_block,       font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height * 3, get_str_width(font_regular, str_player_chunk,       font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height * 4, get_str_width(font_regular, str_player_direction,   font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height * 5, get_str_width(font_regular, str_block_count,        font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height * 6, get_str_width(font_regular, str_quad_count,         font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height * 7, get_str_width(font_regular, str_tri_count,          font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height * 8, get_str_width(font_regular, str_vertex_count,       font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));

    draw_text(font_regular, str_fps,                (v2i16){MARGIN, MARGIN},                        font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_player_pos,         (v2i16){MARGIN, MARGIN + text_row_height},      font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_player_block,       (v2i16){MARGIN, MARGIN + text_row_height * 2},  font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_player_chunk,       (v2i16){MARGIN, MARGIN + text_row_height * 3},  font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_player_direction,   (v2i16){MARGIN, MARGIN + text_row_height * 4},  font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_block_count,        (v2i16){MARGIN, MARGIN + text_row_height * 5},  font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_quad_count,         (v2i16){MARGIN, MARGIN + text_row_height * 6},  font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_tri_count,          (v2i16){MARGIN, MARGIN + text_row_height * 7},  font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);
    draw_text(font_regular, str_vertex_count,       (v2i16){MARGIN, MARGIN + text_row_height * 8},  font_size_debug_info, 1, 0, 0, COL_TEXT_DEFAULT);

    BeginMode3D(*camera);
    rlBegin(RL_LINES);
    draw_line_3d(v3izero, (v3i32){1, 0, 0}, COL_X);
    draw_line_3d(v3izero, (v3i32){0, 1, 0}, COL_Y);
    draw_line_3d(v3izero, (v3i32){0, 0, 1}, COL_Z);
    rlEnd();
    EndMode3D();
}

// raylib/rtext.c/DrawTextEx refactored
void draw_text(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, u8 align_x, u8 align_y, Color tint) {
    // spacing: char spacing;
    // align_x: 0 = left, 1 = center, 2 = right;
    // align_y: 0 = top, 1 = center, 2 = bottom;

    switch (align_x) {
        case 1:
            pos.x -= (get_str_width(font, str, font_size, spacing) / 2);
            break;

        case 2:
            pos.x -= get_str_width(font, str, font_size, spacing);
            break;
    };

    switch (align_y) {
        case 1:
            pos.y -= (font_size / 1.8f);
            break;

        case 2:
            pos.y -= font_size;
            break;
    };

    if (font.texture.id == 0) font = GetFontDefault();
    u16 size = TextLength(str);

    f32 text_offset_y = 0;
    f32 text_offset_x = 0.0f;
    f32 scale_factor = font_size / font.baseSize;
    for (u16 i = 0; i < size;) {
        i32 codepoint_byte_count = 0;
        u16 codepoint = GetCodepointNext(&str[i], &codepoint_byte_count);
        u8 index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n') {
            text_offset_y += (font_size + 2);
            text_offset_x = 0.0f;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                DrawTextCodepoint(font_bold, codepoint,
                        (Vector2){pos.x + text_offset_x + 1, pos.y + text_offset_y + 1},
                        font_size, ColorAlpha(ColorBrightness(tint, -0.1f), 0.4f));

                DrawTextCodepoint(font, codepoint,
                        (Vector2){pos.x + text_offset_x, pos.y + text_offset_y},
                        font_size, tint);
            }

            if (font.glyphs[index].advanceX == 0)
                text_offset_x += ((f32)font.recs[index].width * scale_factor + spacing);
            else
                text_offset_x += ((f32)font.glyphs[index].advanceX * scale_factor + spacing);
        }
        i += codepoint_byte_count;
    }
}

float get_str_width(Font font, const str* str, f32 font_size, f32 spacing) {
    f32 result = 0;
    f32 text_offset_x = 0.0f;
    f32 scale_factor = font_size / font.baseSize;
    for (u16 i = 0; i < TextLength(str);) {
        i32 codepoint_byte_count = 0;
        u16 codepoint = GetCodepointNext(&str[i], &codepoint_byte_count);
        u8 index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n')
            text_offset_x = 0.0f;
        else {
            if (font.glyphs[index].advanceX == 0) {
                result += font.recs[index].width * scale_factor + spacing;
                text_offset_x += ((f32)font.recs[index].width * scale_factor + spacing);
            } else {
                result += font.glyphs[index].advanceX * scale_factor + spacing;
                text_offset_x += ((f32)font.glyphs[index].advanceX * scale_factor + spacing);
            }
        }
        i += codepoint_byte_count;
    }
    return result + 4;
}

// raylib/rtextures.c/DrawTexturePro refactored
void draw_texture(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, u8 align_x, u8 align_y, Color tint) {
    // scale is based on (source.scale * scl)

    if ((texture.id <= 0) || (scl.x <= 0.0f) || (scl.y <= 0.0f)) return;
    if ((source.width == 0.0f) || (source.height == 0.0f)) return;
    // align_x: 0 = left, 1 = center, 2 = right;
    // align_y: 0 = top, 1 = center, 2 = bottom;

    switch (align_x) {
        case 1:
            pos.x -= ((source.width * scl.x) / 2);
            break;

        case 2:
            pos.x -= (source.width * scl.x);
            break;
    };

    switch (align_y) {
        case 1:
            pos.y -= ((source.height * scl.y) / 2);
            break;

        case 2:
            pos.y -= (source.height * scl.y);
            break;
    };

    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0, 0, 1);

    i32 tile_width = source.width * scl.x;
    i32 tile_height = source.height * scl.y;

    // top left
    rlTexCoord2f(source.x / texture.width, source.y / texture.height);
    rlVertex2f(pos.x, pos.y);

    // bottom left
    rlTexCoord2f(source.x / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x, pos.y + tile_height);

    // bottom right
    rlTexCoord2f((source.x + source.width) / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y + tile_height);

    // top right
    rlTexCoord2f((source.x + source.width) / texture.width, source.y / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y);
}

//jump
// TODO: make draw_texture_tiled()
// raylib/examples/textures/textures_draw_tiled.c/DrawTextureTiled refactored
/*
void draw_texture_tiled(Texture2D texture, Rectangle source, Rectangle dest, v2i16 pos, v2i16 scl, Color tint) {
    if ((texture.id <= 0) || (scl.x <= 0.0f) || (scl.y <= 0.0f)) return;
    if ((source.width == 0) || (source.height == 0)) return;
    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0, 0, 1);

    i32 tile_width = source.width*scl.x;
    i32 tile_height = source.height*scl.y;

    // top left
    rlTexCoord2f(source.x/texture.width, source.y/texture.height);
    rlVertex2f(pos.x, pos.y);

    // bottom left
    rlTexCoord2f(source.x/texture.width, (source.y + source.height)/texture.height);
    rlVertex2f(pos.x, pos.y + tile_height);
    
    // bottom right
    rlTexCoord2f((source.x + source.width)/texture.width, (source.y + source.height)/texture.height);
    rlVertex2f(pos.x + tile_width, pos.y + tile_height);

    // top right
    rlTexCoord2f((source.x + source.width)/texture.width, source.y/texture.height);
    rlVertex2f(pos.x + tile_width, pos.y);


    if ((dest.width < tile_width) && (dest.height < tile_height)) {
        DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tile_width)*source.width, ((float)dest.height/tile_height)*source.height},
                    (Rectangle){dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    } else if (dest.width <= tile_width) {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tile_height < dest.height; dy += tile_height) {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tile_width)*source.width, source.height}, (Rectangle){dest.x, dest.y + dy, dest.width, (float)tile_height}, origin, rotation, tint);
        }

        // Fit last tile
        if (dy < dest.height) {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tile_width)*source.width, ((float)(dest.height - dy)/tile_height)*source.height},
                        (Rectangle){dest.x, dest.y + dy, dest.width, dest.height - dy}, origin, rotation, tint);
        }
    } else if (dest.height <= tile_height) {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tile_width < dest.width; dx += tile_width) {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)dest.height/tile_height)*source.height}, (Rectangle){dest.x + dx, dest.y, (float)tile_width, dest.height}, origin, rotation, tint);
        }

        // Fit last tile
        if (dx < dest.width) {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tile_width)*source.width, ((float)dest.height/tile_height)*source.height},
                        (Rectangle){dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    } else {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tile_width < dest.width; dx += tile_width) {
            int dy = 0;
            for (;dy+tile_height < dest.height; dy += tile_height) {
                DrawTexturePro(texture, source, (Rectangle){dest.x + dx, dest.y + dy, (float)tile_width, (float)tile_height}, origin, rotation, tint);
            }

            if (dy < dest.height) {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)(dest.height - dy)/tile_height)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, (float)tile_width, dest.height - dy}, origin, rotation, tint);
            }
        }

        // Fit last column of tiles
        if (dx < dest.width) {
            int dy = 0;
            for (;dy+tile_height < dest.height; dy += tile_height) {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tile_width)*source.width, source.height},
                        (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, (float)tile_height}, origin, rotation, tint);
            }

            // Draw final tile in the bottom right corner
            if (dy < dest.height) {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tile_width)*source.width, ((float)(dest.height - dy)/tile_height)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
            }
        }
    }
}
*/

// raylib/rtextures.c/DrawTexturePro refactored
void draw_texture_simple(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint) {
    // scale is based on (scl)

    if (texture.id <= 0) return;
    f32 width = (f32)texture.width;
    f32 height = (f32)texture.height;

    Vector2 top_left =       (Vector2){pos.x,            pos.y};
    Vector2 bottom_right =   (Vector2){pos.x + scl.x,    pos.y + scl.y};

    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0.0f, 0.0f, 1.0f);

    rlTexCoord2f(source.x / width, source.y / height);
    rlVertex2f(top_left.x, top_left.y);
    rlTexCoord2f(source.x / width, (source.y + source.height) / height);
    rlVertex2f(top_left.x, bottom_right.y);
    rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
    rlVertex2f(bottom_right.x, bottom_right.y);
    rlTexCoord2f((source.x + source.width) / width, source.y / height);
    rlVertex2f(bottom_right.x, top_left.y);
}

void draw_button(Texture2D texture, Rectangle button, v2i16 pos, u8 align_x, u8 align_y, u8 btn_state, void (*func)(), const str *str) {
    // align_x: 0 = left, 1 = center, 2 = right;
    // align_y: 0 = top, 1 = center, 2 = bottom;

    switch (align_x) {
        case 1:
            pos.x -= ((button.width * setting.gui_scale) / 2);
            break;

        case 2:
            pos.x -= (button.width * setting.gui_scale);
            break;
    };

    switch (align_y) {
        case 1:
            pos.y -= ((button.height * setting.gui_scale) / 2);
            break;

        case 2:
            pos.y -= (button.height * setting.gui_scale);
            break;
    };

    if (buttons[btn_state]) {
        if (cursor.x > pos.x && cursor.x < pos.x + (button.width * setting.gui_scale)
                && cursor.y > pos.y && cursor.y < pos.y + (button.height * setting.gui_scale)) {
            draw_texture(texture, button, pos,
                    (v2i16){setting.gui_scale, setting.gui_scale},
                    0, 0, ColorTint(COL_TEXTURE_DEFAULT, TINT_BUTTON_HOVER));

            if (IsMouseButtonPressed(0))
                func();
        } else {
            draw_texture(texture, button, pos, 
                    (v2i16){setting.gui_scale, setting.gui_scale},
                    0, 0, COL_TEXTURE_DEFAULT);
        }

        if (str) {
            draw_text(font_regular, str,
                    (v2i16){pos.x + ((button.width * setting.gui_scale) / 2), pos.y + ((button.height * setting.gui_scale) / 2)},
                    font_size, 1, align_x, align_y, COL_TEXT_DEFAULT);
        }
    } else draw_texture(texture, button_inactive, pos, 
            (v2i16){setting.gui_scale, setting.gui_scale},
            0, 0, COL_TEXTURE_DEFAULT);
}

void btn_func_singleplayer() {
    menu_index = 0; //TODO: set actual value (MENU_SINGLEPLAYER)
    state_menu_depth = 0; //TODO: set actual value (2)
    check_menu_ready = 0;
    state &= ~STATE_PAUSED; //temp

    init_world("Poop Consistency Tester"); //temp
}

void btn_func_multiplayer() {
    menu_index = MENU_MULTIPLAYER;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_minecraft_c_realms() {
    menu_index = MENU_MINECRAFT_C_REALMS;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_options() {
    menu_index = MENU_OPTIONS;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_options_game() {
    menu_index = MENU_OPTIONS_GAME;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_back_to_game() {
    menu_index = 0;
    state_menu_depth = 0;
    check_menu_ready = 0;
    state &= ~STATE_PAUSED;
    lily.state &= ~STATE_MENU_OPEN;
    lily.container_state = 0;
}

void btn_func_quit() {
    state &= ~STATE_ACTIVE;
}

void btn_func_save_and_quit_to_title() {
    menu_index = MENU_TITLE;
    state_menu_depth = 1;
    check_menu_ready = 0;
    // TODO: save and unload world
    state &= ~STATE_WORLD_LOADED;
    state &= ~STATE_PAUSED;
}

void btn_func_back() {
    menu_layer[state_menu_depth] = 0;
    --state_menu_depth;
    menu_index = menu_layer[state_menu_depth];
    check_menu_ready = 0;
}

