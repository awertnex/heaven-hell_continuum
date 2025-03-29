/* ==== section table ==========================================================
_section_input =================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "h/main.h"
#include "h/setting.h"
#include "dir.c"
#include "h/gui.h"
#include "h/chunking.h"
#include "h/logic.h"
#include "h/assets.h"
#include "h/keymaps.h"
#include "h/logger.h"
#include "h/super_debugger.h"

// ---- variables --------------------------------------------------------------
WindowInfo win =
{
    .scl = {WIDTH, HEIGHT},
};
f64 start_time = 0;
u16 state = 0;
u8 state_menu_depth = 0;
settings setting =
{
    .reach_distance =       SETTING_REACH_DISTANCE_MAX,
    .fov =                  SETTING_FOV_DEFAULT,
    .mouse_sensitivity =    SETTING_MOUSE_SENSITIVITY_DEFAULT,
    .render_distance =      SETTING_RENDER_DISTANCE_DEFAULT,
    .gui_scale =            SETTING_GUI_SCALE_DEFAULT,
};

// ---- signatures -------------------------------------------------------------
static void update_world();
static void update_input(player *player);
static void draw_default_grid();

int main(int argc, char **argv) // ---- game init ------------------------------
{
    snprintf(mc_c_grandpath, strlen(getenv("HOME")) + 14, "%s/minecraft.c/", getenv("HOME"));
    init_grandpath_directory();

    // TODO: make launcher screen instead
    if (argc > 1)
    {
        for (u8 i = 1; i < argc; ++i)
        {
            init_instance_directory(argv[i]);
            if (!(state & STATE_ACTIVE))
                return -1;
        }
    }

    if (ModeDebug)
        printf("DEBUG MODE: ON");

    SetWindowState(FLAG_MSAA_4X_HINT);
    InitWindow(WIDTH, HEIGHT, "minecraft.c");
    SetWindowPosition((GetMonitorWidth(0)/2) - (WIDTH/2), (GetMonitorHeight(0)/2) - (HEIGHT/2));

    if (ModeDebug)
        SetWindowState(FLAG_WINDOW_TOPMOST);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_VSYNC_HINT);
    SetWindowMinSize(200, 150);

    setting.render_distance = SETTING_RENDER_DISTANCE_MAX; //temp

    init_texture_layouts();
    init_textures();
    init_gui();
    init_super_debugger();

    state |= STATE_ACTIVE;
    while (state & STATE_ACTIVE) // ---- game loop -----------------------------
    {
        if (!(state & STATE_WORLD_LOADED))
        {
            BeginDrawing();
            ClearBackground(DARKBROWN); /* TODO: make actual panoramic scene */
            EndDrawing();
            update_menus(&lily.state);
        }
        else update_world();

        if (state & STATE_PAUSED)
        {
            BeginDrawing();
            draw_menu_overlay;
            EndDrawing();

            while (state & STATE_PAUSED && state & STATE_ACTIVE)
            {
                BeginDrawing();
                update_menus(&lily.state);
                EndDrawing();
            }
        }
    }

    // ---- game close ---------------------------------------------------------
    unload_textures();
    free_gui();
    free_super_debugger();
    CloseWindow();
    return 0;
}

void init_world()
{
    hide_cursor;
    center_cursor;

    init_chunking();

    { /*temp*/
        chunk_buf[0][0].pos = (v2i16){0, 0};
        chunk_buf[0][1].pos = (v2i16){0, -1};
        chunk_buf[0][2].pos = (v2i16){0, -2};
        chunk_buf[0][3].pos = (v2i16){-1, 0};
        chunk_buf[0][4].pos = (v2i16){0, 1};
        chunk_buf[0][5].pos = (v2i16){0, 2};
        chunk_buf[0][6].pos = (v2i16){-1, -1};
        chunk_buf[0][7].pos = (v2i16){1, 0};
        parse_chunk_states(&chunk_buf[0][0], 3);
        parse_chunk_states(&chunk_buf[0][1], 2);
        parse_chunk_states(&chunk_buf[0][2], 2);
        parse_chunk_states(&chunk_buf[0][3], 30);
        parse_chunk_states(&chunk_buf[0][4], 2);
        parse_chunk_states(&chunk_buf[0][5], 2);
        parse_chunk_states(&chunk_buf[0][6], 3);
        parse_chunk_states(&chunk_buf[0][7], 2);
    }
    lily.state |= STATE_FALLING; //temp
    lily.state &= ~STATE_PARSE_TARGET;

    if (ModeDebug)
    {
        state |= STATE_DEBUG;
        lily.state |= STATE_FLYING;
    }

    state |= STATE_HUD | STATE_WORLD_LOADED;
    LOGINFO("World Loaded: Poop Consistency Tester");
}

void update_world()
{
    start_time = get_time_ms();
    win.scl = (v2f32){GetRenderWidth(), GetRenderHeight()};

    parse_player_states(&lily);

    if (ModeCollide)
        give_collision_static(&lily, &target_coordinates_feet);

    if (state & STATE_DEBUG)
        give_camera_movements_debug_info(&lily);
    give_camera_movements_player(&lily);

    if (state_menu_depth || state & STATE_SUPER_DEBUG)
        show_cursor;
    else hide_cursor;

    update_input(&lily);

    BeginDrawing();
    ClearBackground(COL_SKYBOX); /* TODO: make actual skybox */
    BeginMode3D(lily.camera);
    { /*temp*/
        draw_chunk(&chunk_buf[0][0], 20);
        draw_chunk(&chunk_buf[0][1], 2);
        draw_chunk(&chunk_buf[0][2], 2);
        draw_chunk(&chunk_buf[0][3], 30);
        draw_chunk(&chunk_buf[0][4], 2);
        draw_chunk(&chunk_buf[0][5], 2);
        draw_chunk(&chunk_buf[0][6], 9);
        draw_chunk(&chunk_buf[0][7], 2);
    }

    if (is_range_within_v3fi(&lily.camera.target,
                (v3i32){-WORLD_SIZE, -WORLD_SIZE, WORLD_BOTTOM},
                (v3i32){WORLD_SIZE, WORLD_SIZE, world_height}))
    {
        if (check_target_delta_position(&lily.camera.target, &lily.previous_target))
            target_chunk = get_chunk(&lily.previous_target, &lily.state, STATE_PARSE_TARGET);

        if (target_chunk != NULL && lily.state & STATE_PARSE_TARGET)
            if (target_chunk->i
                    [lily.previous_target.z - WORLD_BOTTOM]
                    [lily.previous_target.y - (target_chunk->pos.y*CHUNK_SIZE)]
                    [lily.previous_target.x - (target_chunk->pos.x*CHUNK_SIZE)] & NOT_EMPTY)
                draw_block_wires(&lily.previous_target);
    }

    if (ModeDebug)
    {
        /*temp
          draw_block_wires(&target_coordinates_feet);
          printf("feet: %d %d %d\n", target_coordinates_feet.x, target_coordinates_feet.y, target_coordinates_feet.z);
          */
        DrawCubeWiresV(lily.camera.target, (Vector3){1, 1, 1}, GREEN);
        draw_bounding_box(&lily.pos, &lily.scl);
        draw_default_grid();
    }

    EndMode3D();

    if (state & STATE_HUD)
    {
        draw_hud();
        draw_debug_info();
        if (state_menu_depth)
        {
            if (lily.container_state & CONTR_INVENTORY)
                draw_inventory();
        }
    }

    draw_super_debugger();
    EndDrawing();
}

// _section_input ==============================================================
void update_input(player *player)
{
    // ---- jumping ------------------------------------------------------------
    if (IsKeyPressed(BIND_JUMP))
        if (get_double_press(player, BIND_JUMP))
            player->state ^= STATE_FLYING;

    if (IsKeyDown(BIND_JUMP))
    {
        if (player->state & STATE_FLYING)
            player->pos.z += player->movement_speed;

        if (player->state & STATE_CAN_JUMP)
        {
            player->v.z += PLAYER_JUMP_HEIGHT;
            player->state &= ~STATE_CAN_JUMP;
        }
    }

    // ---- sneaking -----------------------------------------------------------
    if (IsKeyDown(BIND_SNEAK))
    {
        if (player->state & STATE_FLYING)
            player->pos.z -= player->movement_speed;
        else
            player->state |= STATE_SNEAKING;
    }

    if (IsKeyUp(BIND_SNEAK))
        player->state &= ~STATE_SNEAKING;

    // ---- sprinting ----------------------------------------------------------
    if (IsKeyDown(BIND_SPRINT) && IsKeyDown(BIND_WALK_FORWARDS))
        player->state |= STATE_SPRINTING;

    if (IsKeyUp(BIND_SPRINT) && IsKeyUp(BIND_WALK_FORWARDS))
        player->state &= ~STATE_SPRINTING;

    // ---- moving -------------------------------------------------------------
    if (IsKeyDown(BIND_STRAFE_LEFT))
    {
        player->pos.x -= player->movement_speed*sinf(player->yaw*DEG2RAD);
        player->pos.y += player->movement_speed*cosf(player->yaw*DEG2RAD);
    }

    if (IsKeyDown(BIND_STRAFE_RIGHT))
    {
        player->pos.x += player->movement_speed*sinf(player->yaw*DEG2RAD);
        player->pos.y -= player->movement_speed*cosf(player->yaw*DEG2RAD);
    }

    if (IsKeyDown(BIND_WALK_BACKWARDS))
    {
        player->pos.x -= player->movement_speed*cosf(player->yaw*DEG2RAD);
        player->pos.y -= player->movement_speed*sinf(player->yaw*DEG2RAD);
    }

    if (IsKeyPressed(BIND_WALK_FORWARDS))
        if (get_double_press(player, BIND_WALK_FORWARDS))
            player->state |= STATE_SPRINTING;

    if (IsKeyDown(BIND_WALK_FORWARDS))
    {
        player->pos.x += player->movement_speed*cosf(player->yaw*DEG2RAD);
        player->pos.y += player->movement_speed*sinf(player->yaw*DEG2RAD);
    }

    // ---- gameplay -----------------------------------------------------------
    if (IsMouseButtonDown(BIND_ATTACK_OR_DESTROY))
    {
        if (player->state & STATE_PARSE_TARGET)
        {
            remove_block_state(target_chunk, lily.previous_target.x, lily.previous_target.y, floorf(lily.previous_target.z - WORLD_BOTTOM));
        }
    }

    if (IsMouseButtonDown(BIND_PICK_BLOCK))
    {
    }

    if (IsMouseButtonDown(BIND_USE_ITEM_OR_PLACE_BLOCK))
    {
        add_block_state(target_chunk, lily.previous_target.x, lily.previous_target.y, floorf(lily.previous_target.z - WORLD_BOTTOM));
    }

    // ---- inventory ----------------------------------------------------------
    if (IsKeyPressed(BIND_HOTBAR_SLOT_1) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_1))
        hud_hotbar_slot_selected = 1;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_2) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_2))
        hud_hotbar_slot_selected = 2;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_3) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_3))
        hud_hotbar_slot_selected = 3;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_4) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_4))
        hud_hotbar_slot_selected = 4;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_5) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_5))
        hud_hotbar_slot_selected = 5;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_6) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_6))
        hud_hotbar_slot_selected = 6;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_7) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_7))
        hud_hotbar_slot_selected = 7;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_8) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_8))
        hud_hotbar_slot_selected = 8;

    if (IsKeyPressed(BIND_HOTBAR_SLOT_9) || IsKeyPressed(BIND_HOTBAR_SLOT_KP_9))
        hud_hotbar_slot_selected = 9;

    if (IsKeyPressed(BIND_OPEN_OR_CLOSE_INVENTORY))
    {
        if (player->container_state & CONTR_INVENTORY && state_menu_depth)
        {
            state_menu_depth = 0;
            player->container_state &= ~CONTR_INVENTORY;
        }
        else if (!(player->container_state & CONTR_INVENTORY) && !state_menu_depth)
        {
            state_menu_depth = 1;
            player->container_state |= CONTR_INVENTORY;
        }

        if (!(player->container_state & CONTR_INVENTORY) && state_menu_depth)
            --state_menu_depth;
    }

    // ---- miscellaneous ------------------------------------------------------
    if (IsKeyPressed(BIND_TOGGLE_HUD))
        state ^= STATE_HUD;

    if (IsKeyPressed(BIND_TOGGLE_DEBUG))
        state ^= STATE_DEBUG;

    if (IsKeyPressed(BIND_TOGGLE_FULLSCREEN))
    {
        state ^= STATE_FULLSCREEN;
        ToggleBorderlessWindowed();

        if (state & STATE_FULLSCREEN)
            MaximizeWindow();
        else
        {
            RestoreWindow();
            SetWindowSize(WIDTH, HEIGHT);
            SetWindowPosition((GetMonitorWidth(0)/2) - (WIDTH/2), (GetMonitorHeight(0)/2) - (HEIGHT/2));
        }
        apply_render_settings();
    }

    if (IsKeyPressed(BIND_TOGGLE_PERSPECTIVE))
    {
        if (player->perspective < 4)
            ++player->perspective;
        else player->perspective = 0;
    }

    if (IsKeyPressed(BIND_PAUSE))
    {
        if (!state_menu_depth)
        {
            state_menu_depth = 1;
            menu_index = MENU_GAME;
            state |= STATE_PAUSED;
            player->container_state = 0;
            show_cursor;
        }
        else if (state_menu_depth == 1)
        {
            state_menu_depth = 0;
            player->container_state = 0;
        }
        else --state_menu_depth;
    }
    if (!state_menu_depth && !(state & STATE_SUPER_DEBUG))
    {
        hide_cursor;
        center_cursor;
    }

    // ---- debug --------------------------------------------------------------
    if (IsKeyPressed(KEY_TAB))
        state ^= STATE_SUPER_DEBUG;

    if (IsKeyPressed(BIND_QUIT))
        state &= ~STATE_ACTIVE;
}

void draw_default_grid()
{
    rlPushMatrix();
    rlBegin(RL_LINES);
    draw_line_3d((v3i32){-4, -4, 0}, (v3i32){4, -4, 0}, WHITE);
    draw_line_3d((v3i32){-4, -3, 0}, (v3i32){4, -3, 0}, WHITE);
    draw_line_3d((v3i32){-4, -2, 0}, (v3i32){4, -2, 0}, WHITE);
    draw_line_3d((v3i32){-4, -1, 0}, (v3i32){4, -1, 0}, WHITE);
    draw_line_3d((v3i32){-4, 0, 0}, (v3i32){4, 0, 0}, WHITE);
    draw_line_3d((v3i32){-4, 1, 0}, (v3i32){4, 1, 0}, WHITE);
    draw_line_3d((v3i32){-4, 2, 0}, (v3i32){4, 2, 0}, WHITE);
    draw_line_3d((v3i32){-4, 3, 0}, (v3i32){4, 3, 0}, WHITE);
    draw_line_3d((v3i32){-4, 4, 0}, (v3i32){4, 4, 0}, WHITE);
    draw_line_3d((v3i32){-4, -4, 0}, (v3i32){-4, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){-3, -4, 0}, (v3i32){-3, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){-2, -4, 0}, (v3i32){-2, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){-1, -4, 0}, (v3i32){-1, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){0, -4, 0}, (v3i32){0, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){1, -4, 0}, (v3i32){1, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){2, -4, 0}, (v3i32){2, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){3, -4, 0}, (v3i32){3, 4, 0}, RAYWHITE);
    draw_line_3d((v3i32){4, -4, 0}, (v3i32){4, 4, 0}, RAYWHITE);

    draw_line_3d(v3izero, (v3i32){2, 0, 0}, COL_X);
    draw_line_3d(v3izero, (v3i32){0, 2, 0}, COL_Y);
    draw_line_3d(v3izero, (v3i32){0, 0, 2}, COL_Z);
    rlEnd();
    rlPopMatrix();
}
