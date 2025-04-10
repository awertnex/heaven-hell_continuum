#include <stdio.h>
#include <string.h>

#include "dependencies/raylib-5.5/src/raylib.h"
#include "dependencies/raylib-5.5/src/rlgl.h"

#include "h/gui.h"
#include "keymaps.c"
#include "h/chunking.h"
#include "h/logic.h"

// ---- variables --------------------------------------------------------------
Vector2 cursor;
Image mc_c_icon;
Font fontRegular;
Font fontBold;
Font fontItalic;
Font fontBoldItalic;
u8 fontSize = 22;
u8 textRowHeight = 20;

Texture2D textureHUDWidgets;
Texture2D textureContainerInventory;

Rectangle hotbar =          {0, 0, 202, 22};
Rectangle hotbarSelected = {0, 22, 24, 24};
Rectangle hotbarOffhand =  {24, 22, 22, 24};
Rectangle crosshair =       {240, 0, 16, 16};
Rectangle buttonInactive =     {0, 46, 200, 20};
Rectangle button =              {0, 66, 200, 20};
Rectangle buttonSelected =     {0, 86, 200, 20};
Rectangle containerInventory = {0, 0, 176, 184};
Rectangle containerSlotSize = {0, 0, 16, 16};

v2i16 hotbarPosition;
f32 hotbarSlotSelected = 1;
v2i16 crosshairPosition;
u16 gameMenuPosition = HEIGHT/3;
u8 buttonSpacingVertical = 5;
v2i16 containerInventoryPosition;
v2i16 containerInventoryFirstSlotPosition;

// TODO: you know what TODO
u8 *containerInventorySlots[5][9];
u8 *containerInventorySlotsCrafting[5];
u8 *containerInventorySlotsArmor[5];
u8 containerInventorySlotsOffhand;
u8 containerCursorSlot[2];
u8 *hotbarSlots[9][9];

u16 menuIndex;
u16 menuLayer[5] = {0};
u8 buttons[BTN_COUNT];
static b8 check_menu_ready;

// ---- debug info -------------------------------------------------------------
str strFPS[16];
str strPlayerPos[32];
str strPlayerBlock[32];
str strPlayerChunk[32];
str strPlayerDirection[32];
str strBlockCount[32];
str strQuadCount[32];
str strTriCount[32];
str strVertexCount[32];
u8 fontSizeDebugInfo = 22;

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
        printf("layer %1d: %s\n", i, menu_names[menuLayer[i]]);
    }
    putchar('\n');
}

void init_fonts()
{
    fontRegular =       LoadFont("fonts/minecraft_regular.otf");
    fontBold =          LoadFont("fonts/minecraft_bold.otf");
    fontItalic =        LoadFont("fonts/minecraft_italic.otf");
    fontBoldItalic =    LoadFont("fonts/minecraft_bold_italic.otf");
}

void init_gui()
{
    mc_c_icon = LoadImage("resources/logo/128x128.png");
    SetWindowIcon(mc_c_icon);

    textureHUDWidgets =           LoadTexture("resources/gui/widgets.png");
    textureContainerInventory =   LoadTexture("resources/gui/containers/inventory.png");

    menuIndex = MENU_TITLE;
    stateMenuDepth = 1;
    memset(buttons, 0, BTN_COUNT);
}

void apply_render_settings(v2f32 renderSize)
{
    hotbarPosition =
        (v2i16){
            roundf((renderSize.x/2) - ((f32)hotbar.width/2)),
            renderSize.y - hotbar.height - 2,
        };
    crosshairPosition =
        (v2i16){
            (renderSize.x/2) - ((f32)crosshair.width/2),
            (renderSize.y/2) - ((f32)crosshair.height/2),
        };
    containerInventoryPosition = (v2i16){
        roundf((renderSize.x/2) - ((f32)containerInventory.width/2)),
        roundf((renderSize.y/2) - ((f32)containerInventory.height/2)),
    };
}

void free_gui()
{
    UnloadImage(mc_c_icon);
    UnloadFont(fontRegular);
    UnloadFont(fontBold);
    UnloadFont(fontItalic);
    UnloadFont(fontBoldItalic);
    UnloadTexture(textureHUDWidgets);
    UnloadTexture(textureContainerInventory);
}

void update_menus(v2f32 renderSize)
{
    if (!menuIndex)
        return;

    if (IsKeyPressed(bindQuit))
        state &= ~STATE_ACTIVE;

    detect_cursor;
    switch (menuIndex)
    {
        case MENU_TITLE:
            if (!check_menu_ready)
            {
                menuLayer[stateMenuDepth] = MENU_TITLE;
                menuIndex = MENU_TITLE;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_SINGLEPLAYER] = 1;
                buttons[BTN_MULTIPLAYER] = 1;
                buttons[BTN_MINECRAFT_C_REALMS] = 1;
                buttons[BTN_OPTIONS] = 1;
                buttons[BTN_QUIT] = 1;
                check_menu_ready = 1;
            }

            draw_text(fontRegular, MC_C_VERSION,
                    (v2i16){6, renderSize.y - 3},
                    fontSize, 2, 0, 2, COL_TEXT_DEFAULT);

            draw_text(fontRegular, MC_C_AUTHOR,
                    (v2i16){
                    renderSize.x - 2,
                    renderSize.y - 3},
                    fontSize, 2, 2, 2, COL_TEXT_DEFAULT);

            rlBegin(RL_QUADS);

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition},
                    BTN_SINGLEPLAYER,
                    &btn_func_singleplayer,
                    "Singleplayer");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + ((button.height + buttonSpacingVertical)*setting.guiScale)},
                    BTN_MULTIPLAYER,
                    &btn_func_multiplayer,
                    "Multiplayer");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + (((button.height + buttonSpacingVertical)*2)*setting.guiScale)},
                    BTN_MINECRAFT_C_REALMS,
                    &btn_func_minecraft_c_realms,
                    "Minecraft.c Realms");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + (((button.height + buttonSpacingVertical)*3)*setting.guiScale)},
                    BTN_OPTIONS,
                    &btn_func_options,
                    "Options...");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + (((button.height + buttonSpacingVertical)*4)*setting.guiScale)},
                    BTN_QUIT,
                    &btn_func_quit,
                    "Quit Game");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_OPTIONS:
            if (!check_menu_ready)
            {
                menuLayer[stateMenuDepth] = MENU_OPTIONS;
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

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition},
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_OPTIONS_GAME:
            if (!check_menu_ready)
            {
                menuLayer[stateMenuDepth] = MENU_OPTIONS_GAME;
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

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition},
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;


        case MENU_GAME:
            if (!check_menu_ready)
            {
                menuLayer[stateMenuDepth] = MENU_GAME;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_BACK_TO_GAME] = 1;
                buttons[BTN_ADVANCEMENTS] = 1;
                buttons[BTN_GIVE_FEEDBACK] = 1;
                buttons[BTN_OPTIONS_GAME] = 1;
                buttons[BTN_QUIT] = 1;
                check_menu_ready = 1;
            }

            if (IsKeyPressed(bindPause))
                btn_func_back_to_game();

            rlBegin(RL_QUADS);

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition},
                    BTN_BACK_TO_GAME,
                    &btn_func_back_to_game,
                    "Back to Game");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + ((button.height + buttonSpacingVertical)*setting.guiScale)},
                    BTN_ADVANCEMENTS,
                    &btn_func_options_game,
                    "Advancements");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + (((button.height + buttonSpacingVertical)*2)*setting.guiScale)},
                    BTN_GIVE_FEEDBACK,
                    &btn_func_options_game,
                    "Give Feedback");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + (((button.height + buttonSpacingVertical)*3)*setting.guiScale)},
                    BTN_OPTIONS_GAME,
                    &btn_func_options_game,
                    "Options...");

            draw_button(textureHUDWidgets, button,
                    (v2i16){renderSize.x/2, gameMenuPosition + (((button.height + buttonSpacingVertical)*4)*setting.guiScale)},
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

    draw_texture(textureHUDWidgets, hotbar,
            hotbarPosition, 
            (v2i16){
            setting.guiScale,
            setting.guiScale}, COL_TEXTURE_DEFAULT);

    draw_texture(textureHUDWidgets, hotbarSelected,
            (v2i16){
            (hotbarPosition.x - 1) + ((hotbar.height - 2)*(hotbarSlotSelected - 1)),
            hotbarPosition.y - 1}, 
            (v2i16){
            setting.guiScale,
            setting.guiScale}, COL_TEXTURE_DEFAULT);

    draw_texture(textureHUDWidgets, hotbarOffhand,
            (v2i16){
            hotbarPosition.x - (hotbar.height*2),
            hotbarPosition.y}, 
            (v2i16){
            setting.guiScale,
            setting.guiScale}, COL_TEXTURE_DEFAULT);

    if (!(state & STATE_DEBUG))
        draw_texture(textureHUDWidgets, crosshair,
                crosshairPosition, 
                (v2i16){
                setting.guiScale,
                setting.guiScale}, COL_TEXTURE_DEFAULT);

    rlEnd();
    rlSetTexture(0);
}

void draw_inventory(v2f32 renderSize)
{
    draw_menu_overlay(renderSize);
    rlBegin(RL_QUADS);

    draw_texture(textureContainerInventory,
            containerInventory,
            containerInventoryPosition, 
            (v2i16){
            setting.guiScale,
            setting.guiScale}, COL_TEXTURE_DEFAULT);

    rlEnd();
    rlSetTexture(0);
}

void draw_debug_info(Camera3D *camera)
{
    if (!(state & STATE_DEBUG)) return;

    update_debug_strings();

    // TODO: rewrite DrawRectangle, get rectangle correct size for font
    DrawRectangle(MARGIN - 2, MARGIN,                     get_str_width(fontRegular, strFPS,                fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight,     get_str_width(fontRegular, strPlayerPos,          fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight*2,   get_str_width(fontRegular, strPlayerBlock,        fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight*3,   get_str_width(fontRegular, strPlayerChunk,        fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight*4,   get_str_width(fontRegular, strPlayerDirection,    fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight*5,   get_str_width(fontRegular, strBlockCount,         fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight*6,   get_str_width(fontRegular, strQuadCount,          fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight*7,   get_str_width(fontRegular, strTriCount,           fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));
    DrawRectangle(MARGIN - 2, MARGIN + textRowHeight*8,   get_str_width(fontRegular, strVertexCount,        fontSizeDebugInfo, 1), textRowHeight, color(255, 255, 255, 100, 40));

    draw_text(fontRegular, strFPS,              (v2i16){MARGIN, MARGIN},                    fontSizeDebugInfo, 1, 0, 0, COL_STATS_1);
    draw_text(fontRegular, strPlayerPos,        (v2i16){MARGIN, MARGIN + textRowHeight},    fontSizeDebugInfo, 1, 0, 0, COL_STATS_2);
    draw_text(fontRegular, strPlayerBlock,      (v2i16){MARGIN, MARGIN + textRowHeight*2},  fontSizeDebugInfo, 1, 0, 0, COL_STATS_2);
    draw_text(fontRegular, strPlayerChunk,      (v2i16){MARGIN, MARGIN + textRowHeight*3},  fontSizeDebugInfo, 1, 0, 0, COL_STATS_2);
    draw_text(fontRegular, strPlayerDirection,  (v2i16){MARGIN, MARGIN + textRowHeight*4},  fontSizeDebugInfo, 1, 0, 0, COL_STATS_2);
    draw_text(fontRegular, strBlockCount,       (v2i16){MARGIN, MARGIN + textRowHeight*5},  fontSizeDebugInfo, 1, 0, 0, COL_STATS_3);
    draw_text(fontRegular, strQuadCount,        (v2i16){MARGIN, MARGIN + textRowHeight*6},  fontSizeDebugInfo, 1, 0, 0, COL_STATS_3);
    draw_text(fontRegular, strTriCount,         (v2i16){MARGIN, MARGIN + textRowHeight*7},  fontSizeDebugInfo, 1, 0, 0, COL_STATS_3);
    draw_text(fontRegular, strVertexCount,      (v2i16){MARGIN, MARGIN + textRowHeight*8},  fontSizeDebugInfo, 1, 0, 0, COL_STATS_3);

    BeginMode3D(*camera);
    rlBegin(RL_LINES);
    draw_line_3d(v3izero, (v3i32){1, 0, 0}, COL_X);
    draw_line_3d(v3izero, (v3i32){0, 1, 0}, COL_Y);
    draw_line_3d(v3izero, (v3i32){0, 0, 1}, COL_Z);
    rlEnd();
    EndMode3D();
}

// raylib/rtext.c/DrawTextEx refactored
void draw_text(Font font, const str *str, v2i16 pos, f32 font_size, f32 spacing, u8 alignX, u8 alignY, Color tint)
{
    // spacing: char spacing;
    // alignX: 0 = left, 1 = center, 2 = right;
    // alignY: 0 = top, 1 = center, 2 = bottom;

    switch (alignX)
    {
        case 1:
            pos.x -= (get_str_width(font, str, font_size, spacing)/2);
            break;

        case 2:
            pos.x -= get_str_width(font, str, font_size, spacing);
            break;
    };

    switch (alignY)
    {
        case 1:
            pos.y -= (font_size/1.8f);
            break;

        case 2:
            pos.y -= font_size;
            break;
    };

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
                DrawTextCodepoint(fontBold, codepoint,
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
    pos.x -= (button.width*setting.guiScale)/2;
    pos.y -= (button.height*setting.guiScale)/2;

    if (buttons[btn_state])
    {
        if (cursor.x > pos.x && cursor.x < pos.x + (button.width*setting.guiScale)
                && cursor.y > pos.y && cursor.y < pos.y + (button.height*setting.guiScale))
        {
            draw_texture(texture, button, pos,
                    (v2i16){setting.guiScale, setting.guiScale},
                    ColorTint(COL_TEXTURE_DEFAULT, TINT_BUTTON_HOVER));

            if (IsMouseButtonPressed(0))
                func();
        }
        else
        {
            draw_texture(texture, button, pos, 
                    (v2i16){setting.guiScale, setting.guiScale},
                    COL_TEXTURE_DEFAULT);
        }

        if (str)
        {
            draw_text(fontRegular, str,
                    (v2i16){pos.x + (button.width/2), pos.y + (button.height/2)},
                    button.height*0.7f, 1, 1, 1, COL_TEXT_DEFAULT);
        }
    }
    else draw_texture(texture, buttonInactive, pos, 
            (v2i16){
            setting.guiScale,
            setting.guiScale}, COL_TEXTURE_DEFAULT);
}

void btn_func_singleplayer()
{
    menuIndex = 0; //TODO: set actual value (MENU_SINGLEPLAYER)
    stateMenuDepth = 0; //TODO: set actual value (2)
    check_menu_ready = 0;
    state &= ~STATE_PAUSED;

    init_world(); //temp
}

void btn_func_multiplayer()
{
    menuIndex = MENU_MULTIPLAYER;
    stateMenuDepth = 2;
    check_menu_ready = 0;
}

void btn_func_minecraft_c_realms()
{
    menuIndex = MENU_MINECRAFT_C_REALMS;
    stateMenuDepth = 2;
    check_menu_ready = 0;
}

void btn_func_options()
{
    menuIndex = MENU_OPTIONS;
    stateMenuDepth = 2;
    check_menu_ready = 0;
}

void btn_func_options_game()
{
    menuIndex = MENU_OPTIONS_GAME;
    stateMenuDepth = 2;
    check_menu_ready = 0;
}

void btn_func_back_to_game()
{
    menuIndex = 0;
    stateMenuDepth = 0;
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
    menuIndex = MENU_TITLE;
    stateMenuDepth = 1;
    check_menu_ready = 0;
    // TODO: save and unload world
    state &= ~STATE_WORLD_LOADED;
    state &= ~STATE_PAUSED;
}

void btn_func_back()
{
    menuLayer[stateMenuDepth] = 0;
    --stateMenuDepth;
    menuIndex = menuLayer[stateMenuDepth];
    check_menu_ready = 0;
}
