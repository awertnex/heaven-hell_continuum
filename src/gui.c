#include <stdio.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "h/gui.h"
#include "h/keymaps.h"
#include "h/main.h"
#include "h/chunking.h"
#include "h/logic.h"

// ---- variables --------------------------------------------------------------
Vector2 cursor;
Font font_regular;
Font font_bold;
Font font_italic;
Font font_bold_italic;
u8 font_size = 22;
u8 text_row_height = 20;

Texture2D texture_hud_widgets;
Texture2D texture_container_inventory;

Rectangle hud_hotbar =          {0, 0, 202, 22};
Rectangle hud_hotbar_selected = {0, 22, 24, 24};
Rectangle hud_hotbar_offhand =  {24, 22, 22, 24};
Rectangle hud_crosshair =       {240, 0, 16, 16};
Rectangle button_inactive =     {0, 46, 200, 20};
Rectangle button =              {0, 66, 200, 20};
Rectangle button_selected =     {0, 86, 200, 20};
Rectangle container_inventory = {0, 0, 176, 184};
Rectangle container_slot_size = {0, 0, 16, 16};

v2i16 hud_hotbar_position;
f32 hud_hotbar_slot_selected = 1;
v2i16 hud_crosshair_position;
u16 game_menu_position = HEIGHT/3;
u8 button_spacing_verical = 5;
v2i16 container_inventory_position;
v2i16 container_inventory_first_slot_position;

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
str str_player_position[32];
str str_player_block[32];
str str_player_chunk[32];
str str_player_direction[32];
str str_block_count[32];
str str_quad_count[32];
str str_tri_count[32];
str str_vertex_count[32];
u8 font_size_debug_info = 22;
Camera3D camera_debug_info =
{
    .up.z = 1,
    .fovy = 50,
    .projection = CAMERA_ORTHOGRAPHIC,
};

// ---- functions --------------------------------------------------------------
static void print_menu_layers()
{
    static str menu_names[10][24] =
    {
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
    for (u8 i = 0; i < 9; ++i)
    {
        printf("layer %1d: %s\n", i, menu_names[menu_layer[i]]);
    }
    putchar('\n');
}

void init_fonts()
{
    font_regular =                  LoadFont("fonts/minecraft_regular.otf");
    font_bold =                     LoadFont("fonts/minecraft_bold.otf");
    font_italic =                   LoadFont("fonts/minecraft_italic.otf");
    font_bold_italic =              LoadFont("fonts/minecraft_bold_italic.otf");
}

void init_gui()
{
    texture_hud_widgets =           LoadTexture("resources/gui/widgets.png");
    texture_container_inventory =   LoadTexture("resources/gui/containers/inventory.png");

    menu_index = MENU_TITLE;
    state_menu_depth = 1;
    memset(buttons, 0, BTN_COUNT);
    apply_render_settings();
}

void apply_render_settings()
{
    hud_hotbar_position =
        (v2i16){
            roundf((win.scl.x/2) - ((f32)hud_hotbar.width/2)),
            win.scl.y - hud_hotbar.height - 2,
        };
    hud_crosshair_position =
        (v2i16){
            (win.scl.x/2) - ((f32)hud_crosshair.width/2),
            (win.scl.y/2) - ((f32)hud_crosshair.height/2),
        };
    container_inventory_position = (v2i16){
        roundf((win.scl.x/2) - ((f32)container_inventory.width/2)),
        roundf((win.scl.y/2) - ((f32)container_inventory.height/2)),
    };
}

void free_gui()
{
    UnloadFont(font_regular);
    UnloadFont(font_bold);
    UnloadFont(font_italic);
    UnloadFont(font_bold_italic);
    UnloadTexture(texture_hud_widgets);
    UnloadTexture(texture_container_inventory);
}

void update_menus()
{
    if (!menu_index)
        return;

    if (IsKeyPressed(BIND_QUIT))
        state &= ~STATE_ACTIVE;

    detect_cursor;
    switch (menu_index)
    {
        case MENU_TITLE:
            if (!check_menu_ready)
            {
                menu_layer[state_menu_depth] = MENU_TITLE;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_SINGLEPLAYER] = 1;
                buttons[BTN_MULTIPLAYER] = 1;
                buttons[BTN_MINECRAFT_C_REALMS] = 1;
                buttons[BTN_OPTIONS] = 1;
                buttons[BTN_QUIT] = 1;
                check_menu_ready = 1;
            }

            draw_text(font_regular, MC_C_VERSION,
                    (v2i16){0, win.scl.y - font_size},
                    font_size, 2, COL_TEXT_DEFAULT);

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position},
                    BTN_SINGLEPLAYER,
                    &btn_func_singleplayer,
                    "Singleplayer");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + ((button.height + button_spacing_verical)*setting.gui_scale)},
                    BTN_MULTIPLAYER,
                    &btn_func_multiplayer,
                    "Multiplayer");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + (((button.height + button_spacing_verical)*2)*setting.gui_scale)},
                    BTN_MINECRAFT_C_REALMS,
                    &btn_func_minecraft_c_realms,
                    "Minecraft.c Realms");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + (((button.height + button_spacing_verical)*3)*setting.gui_scale)},
                    BTN_OPTIONS,
                    &btn_func_options,
                    "Options...");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + (((button.height + button_spacing_verical)*4)*setting.gui_scale)},
                    BTN_QUIT,
                    &btn_func_quit,
                    "Quit Game");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_OPTIONS:
            if (!check_menu_ready)
            {
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
                    (v2i16){win.scl.x/2, game_menu_position},
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_OPTIONS_GAME:
            if (!check_menu_ready)
            {
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
                    (v2i16){win.scl.x/2, game_menu_position},
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;


        case MENU_GAME:
            if (!check_menu_ready)
            {
                menu_layer[state_menu_depth] = MENU_GAME;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_BACK_TO_GAME] = 1;
                buttons[BTN_ADVANCEMENTS] = 1;
                buttons[BTN_GIVE_FEEDBACK] = 1;
                buttons[BTN_OPTIONS_GAME] = 1;
                buttons[BTN_QUIT] = 1;
                check_menu_ready = 1;
            }

            if (IsKeyPressed(BIND_PAUSE))
                btn_func_back_to_game();

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position},
                    BTN_BACK_TO_GAME,
                    &btn_func_back_to_game,
                    "Back to Game");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + ((button.height + button_spacing_verical)*setting.gui_scale)},
                    BTN_ADVANCEMENTS,
                    &btn_func_options_game,
                    "Advancements");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + (((button.height + button_spacing_verical)*2)*setting.gui_scale)},
                    BTN_GIVE_FEEDBACK,
                    &btn_func_options_game,
                    "Give Feedback");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + (((button.height + button_spacing_verical)*3)*setting.gui_scale)},
                    BTN_OPTIONS_GAME,
                    &btn_func_options_game,
                    "Options...");

            draw_button(texture_hud_widgets, button,
                    (v2i16){win.scl.x/2, game_menu_position + (((button.height + button_spacing_verical)*4)*setting.gui_scale)},
                    BTN_QUIT,
                    &btn_func_save_and_quit_to_title,
                    "Save and Quit to Title");

            rlEnd();
            rlSetTexture(0);
            break;
    }
}

void draw_hud()
{
    rlBegin(RL_QUADS);

    draw_texture(texture_hud_widgets, hud_hotbar,
            hud_hotbar_position, 
            (v2i16){
            setting.gui_scale,
            setting.gui_scale}, COL_TEXTURE_DEFAULT);

    draw_texture(texture_hud_widgets, hud_hotbar_selected,
            (v2i16){
            (hud_hotbar_position.x - 1) + ((hud_hotbar.height - 2)*(hud_hotbar_slot_selected - 1)),
            hud_hotbar_position.y - 1}, 
            (v2i16){
            setting.gui_scale,
            setting.gui_scale}, COL_TEXTURE_DEFAULT);

    draw_texture(texture_hud_widgets, hud_hotbar_offhand,
            (v2i16){
            hud_hotbar_position.x - (hud_hotbar.height*2),
            hud_hotbar_position.y}, 
            (v2i16){
            setting.gui_scale,
            setting.gui_scale}, COL_TEXTURE_DEFAULT);

    if (!(state & STATE_DEBUG))
        draw_texture(texture_hud_widgets, hud_crosshair,
                hud_crosshair_position, 
                (v2i16){
                setting.gui_scale,
                setting.gui_scale}, COL_TEXTURE_DEFAULT);

    rlEnd();
    rlSetTexture(0);
}

void draw_inventory()
{
    draw_menu_overlay;
    rlBegin(RL_QUADS);

    draw_texture(texture_container_inventory,
            container_inventory,
            container_inventory_position, 
            (v2i16){
            setting.gui_scale,
            setting.gui_scale}, COL_TEXTURE_DEFAULT);

    rlEnd();
    rlSetTexture(0);
}

void draw_debug_info()
{
    if (!(state & STATE_DEBUG)) return;

    snprintf(str_fps, 16,                       "FPS: %d",                  GetFPS());
    snprintf(str_player_position, 32,           "XYZ: %.2f %.2f %.2f",      lily.pos.x, lily.pos.y, lily.pos.z);
    snprintf(str_player_block, 32,              "BLOCK: %.0f %.0f %.0f",    floorf(lily.pos.x), floorf(lily.pos.y), floorf(lily.pos.z));
    snprintf(str_player_chunk, 32,              "CHUNK: %d %d",             (i16)floorf(lily.pos.x/CHUNK_SIZE), (i16)floorf(lily.pos.y/CHUNK_SIZE));
    snprintf(str_player_direction, 32,          "YAW: %.1f PITCH: %.1f",    lily.yaw, lily.pitch);
    snprintf(str_block_count, 32,               "BLOCKS: %ld",              block_count);
    snprintf(str_quad_count, 32,                "QUADS: %ld",               quad_count);
    snprintf(str_tri_count, 32,                 "TRIS: %ld",                quad_count*2);
    snprintf(str_vertex_count, 32,              "VERTICES: %ld",            quad_count*6);

    // TODO: rewrite DrawRectangle, get rectangle correct size for font
    DrawRectangle(MARGIN - 2, MARGIN,                       get_str_width(font_regular, str_fps,                font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height,     get_str_width(font_regular, str_player_position,    font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height*2,   get_str_width(font_regular, str_player_block,       font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height*3,   get_str_width(font_regular, str_player_chunk,       font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height*4,   get_str_width(font_regular, str_player_direction,   font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height*5,   get_str_width(font_regular, str_block_count,        font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height*6,   get_str_width(font_regular, str_quad_count,         font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height*7,   get_str_width(font_regular, str_tri_count,          font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + text_row_height*8,   get_str_width(font_regular, str_vertex_count,       font_size_debug_info, 1), text_row_height, color(255, 255, 255, 100, 40));

    draw_text(font_regular, str_fps,                (v2i16){MARGIN, MARGIN},                        font_size_debug_info, 1, COL_STATS_1);
    draw_text(font_regular, str_player_position,    (v2i16){MARGIN, MARGIN + text_row_height},      font_size_debug_info, 1, COL_STATS_2);
    draw_text(font_regular, str_player_block,       (v2i16){MARGIN, MARGIN + text_row_height*2},    font_size_debug_info, 1, COL_STATS_2);
    draw_text(font_regular, str_player_chunk,       (v2i16){MARGIN, MARGIN + text_row_height*3},    font_size_debug_info, 1, COL_STATS_2);
    draw_text(font_regular, str_player_direction,   (v2i16){MARGIN, MARGIN + text_row_height*4},    font_size_debug_info, 1, COL_STATS_2);
    draw_text(font_regular, str_block_count,        (v2i16){MARGIN, MARGIN + text_row_height*5},    font_size_debug_info, 1, COL_STATS_3);
    draw_text(font_regular, str_quad_count,         (v2i16){MARGIN, MARGIN + text_row_height*6},    font_size_debug_info, 1, COL_STATS_3);
    draw_text(font_regular, str_tri_count,          (v2i16){MARGIN, MARGIN + text_row_height*7},    font_size_debug_info, 1, COL_STATS_3);
    draw_text(font_regular, str_vertex_count,       (v2i16){MARGIN, MARGIN + text_row_height*8},    font_size_debug_info, 1, COL_STATS_3);

    BeginMode3D(camera_debug_info);
    rlBegin(RL_LINES);
    draw_line_3d(v3izero, (v3i32){1, 0, 0}, COL_X);
    draw_line_3d(v3izero, (v3i32){0, 1, 0}, COL_Y);
    draw_line_3d(v3izero, (v3i32){0, 0, 1}, COL_Z);
    rlEnd();
    EndMode3D();
}

// raylib/rtext.c/DrawTextEx refactored
void draw_text(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, Color tint)
{
    if (font.texture.id == 0) font = GetFontDefault();
    u16 size = TextLength(str);

    f32 textOffsetY = 0;
    f32 textOffsetX = 0.0f;
    f32 scaleFactor = font_size/font.baseSize;
    for (u16 i = 0; i < size;)
    {
        i32 codepointByteCount = 0;
        u16 codepoint = GetCodepointNext(&str[i], &codepointByteCount);
        u8 index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n')
        {
            textOffsetY += (font_size + 2);
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint(font_bold, codepoint,
                        (Vector2){pos.x + textOffsetX + 1, pos.y + textOffsetY + 1},
                        font_size, ColorAlpha(ColorBrightness(tint, -0.1f), 0.4f));

                DrawTextCodepoint(font, codepoint,
                        (Vector2){pos.x + textOffsetX, pos.y + textOffsetY},
                        font_size, tint);
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += ((f32)font.recs[index].width*scaleFactor + spacing);
            else
                textOffsetX += ((f32)font.glyphs[index].advanceX*scaleFactor + spacing);
        }
        i += codepointByteCount;
    }
}

// raylib/rtext.c/DrawTextEx refactored
void draw_text_centered(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, Color tint, u8 center_vertically)
{
    pos.x -= (get_str_width(font, str, font_size, spacing)/2);

    if (center_vertically)
        pos.y -= (font_size/1.8f);

    draw_text(font, str, pos, font_size, spacing, tint);
}

float get_str_width(Font font, const str *str, f32 font_size, f32 spacing)
{
    f32 result = 0;
    f32 textOffsetX = 0.0f;
    f32 scaleFactor = font_size/font.baseSize;
    for (u16 i = 0; i < TextLength(str);)
    {
        i32 codepointByteCount = 0;
        u16 codepoint = GetCodepointNext(&str[i], &codepointByteCount);
        u8 index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n')
            textOffsetX = 0.0f;
        else
        {
            if (font.glyphs[index].advanceX == 0)
            {
                result += font.recs[index].width*scaleFactor + spacing;
                textOffsetX += ((f32)font.recs[index].width*scaleFactor + spacing);
            }
            else
            {
                result += font.glyphs[index].advanceX*scaleFactor + spacing;
                textOffsetX += ((f32)font.glyphs[index].advanceX*scaleFactor + spacing);
            }
        }
        i += codepointByteCount;
    }
    return result + 4;
}

// raylib/rtextures.c/DrawTexturePro refactored
void draw_texture(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint) /* scale is based on source.scale*scl */
{
    if (texture.id <= 0) return;
    f32 width = (f32)texture.width;
    f32 height = (f32)texture.height;

    Vector2 topLeft =       (Vector2){pos.x,                        pos.y};
    Vector2 bottomRight =   (Vector2){pos.x + (source.width*scl.x), pos.y + (source.height*scl.y)};

    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0, 0, 1);

    rlTexCoord2f(source.x/width, source.y/height);
    rlVertex2f(topLeft.x, topLeft.y);
    rlTexCoord2f(source.x/width, (source.y + source.height)/height);
    rlVertex2f(topLeft.x, bottomRight.y);
    rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
    rlVertex2f(bottomRight.x, bottomRight.y);
    rlTexCoord2f((source.x + source.width)/width, source.y/height);
    rlVertex2f(bottomRight.x, topLeft.y);
}

// raylib/rtextures.c/DrawTexturePro refactored
void draw_texture_simple(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint) /* scale is based on scl */
{
    if (texture.id <= 0) return;
    f32 width = (f32)texture.width;
    f32 height = (f32)texture.height;

    Vector2 topLeft =       (Vector2){pos.x,            pos.y};
    Vector2 bottomRight =   (Vector2){pos.x + scl.x,    pos.y + scl.y};

    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0, 0, 1);

    rlTexCoord2f(source.x/width, source.y/height);
    rlVertex2f(topLeft.x, topLeft.y);
    rlTexCoord2f(source.x/width, (source.y + source.height)/height);
    rlVertex2f(topLeft.x, bottomRight.y);
    rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
    rlVertex2f(bottomRight.x, bottomRight.y);
    rlTexCoord2f((source.x + source.width)/width, source.y/height);
    rlVertex2f(bottomRight.x, topLeft.y);
}

void draw_button(Texture2D texture, Rectangle button, v2i16 pos, u8 btn_state, void (*func)(), const str *str)
{
    pos.x -= (button.width*setting.gui_scale)/2;
    pos.y -= (button.height*setting.gui_scale)/2;
    v2i16 text_offset = {pos.x + button.width/2, pos.y + button.height/2};

    if (buttons[btn_state])
    {
        if (cursor.x > pos.x && cursor.x < pos.x + (button.width*setting.gui_scale)
                && cursor.y > pos.y && cursor.y < pos.y + (button.height*setting.gui_scale))
        {
            draw_texture(texture, button, pos,
                    (v2i16){setting.gui_scale, setting.gui_scale},
                    ColorTint(COL_TEXTURE_DEFAULT, TINT_BUTTON_HOVER));

            if (IsMouseButtonPressed(0))
                func();
        }
        else
        {
            draw_texture(texture, button, pos, 
                    (v2i16){setting.gui_scale, setting.gui_scale},
                    COL_TEXTURE_DEFAULT);
        }

        if (str) draw_text_centered(font_regular, str, text_offset, button.height*0.7f, 1, COL_TEXT_DEFAULT, 1);
    }
    else draw_texture(texture, button_inactive, pos, 
            (v2i16){
            setting.gui_scale,
            setting.gui_scale}, COL_TEXTURE_DEFAULT);
}

void btn_func_singleplayer()
{
    menu_index = 0; //TODO: set actual value (MENU_SINGLEPLAYER)
    state_menu_depth = 0; //TODO: set actual value (2)
    check_menu_ready = 0;
    state &= ~STATE_PAUSED;

    init_world(); //temp
}

void btn_func_multiplayer()
{
    menu_index = MENU_MULTIPLAYER;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_minecraft_c_realms()
{
    menu_index = MENU_MINECRAFT_C_REALMS;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_options()
{
    menu_index = MENU_OPTIONS;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_options_game()
{
    menu_index = MENU_OPTIONS_GAME;
    state_menu_depth = 2;
    check_menu_ready = 0;
}

void btn_func_back_to_game()
{
    menu_index = 0;
    state_menu_depth = 0;
    check_menu_ready = 0;
    state &= ~STATE_PAUSED;
    lily.state &= ~STATE_MENU_OPEN;
}

void btn_func_quit()
{
    state &= ~STATE_ACTIVE;
}

void btn_func_save_and_quit_to_title()
{
    menu_index = MENU_TITLE;
    state_menu_depth = 1;
    check_menu_ready = 0;
    // TODO: save and unload world
    state &= ~STATE_WORLD_LOADED;
    ClearBackground(DARKBROWN);
}

void btn_func_back()
{
    --state_menu_depth;
    menu_index = menu_layer[state_menu_depth];
    check_menu_ready = 0;
}
