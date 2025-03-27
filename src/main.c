/* ==== section table ==========================================================
_section_instance_directory_map ================================================
_section_input =================================================================
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "h/main.h"
#include "h/setting.h"
#include "h/gui.h"
#include "h/chunking.h"
#include "h/logic.h"
#include "h/assets.h"
#include "h/keymaps.h"
#include "h/super_debugger.h"

// ---- variables --------------------------------------------------------------
window win =
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
static void init_game();
static void update_game();
static void close_game();
void update_input(player *player);
void update_game_menu(player *player);
void draw_default_grid();

// _section_instance_directory_map =============================================
FILE *instance;
void init_instance_dir(str **instance_name)
{
    if (mkdir(*instance_name, 0775))
    {
        instance = fopen(*instance_name, "r+");
        printf("Instance Opened: %s\n", *instance_name);
    }
    else
    {
        instance = fopen(*instance_name, "r+");
        printf("Instance Created: %s\n", *instance_name);
    }

    if (instance != 0) fclose(instance);
}

void init_game()
{
    if (ModeDebug)
        printf("DEBUG MODE: ON");

    SetWindowState(FLAG_WINDOW_HIGHDPI);
    SetWindowState(FLAG_MSAA_4X_HINT);
    InitWindow(WIDTH, HEIGHT, "minecraft.c");
    SetWindowPosition((GetMonitorWidth(0)/2) - (WIDTH/2), (GetMonitorHeight(0)/2) - (HEIGHT/2));
    //TODO: fix fullscreen

    if (ModeDebug)
        SetWindowState(FLAG_WINDOW_TOPMOST);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_VSYNC_HINT);
    SetWindowMinSize(200, 150);
    hide_cursor;
    center_cursor;

    setting.render_distance = SETTING_RENDER_DISTANCE_MAX; //temp

    init_chunking();
    init_texture_layouts();
    init_textures();
    init_gui();
    init_super_debugger();

    { /*temp*/
        chunk_buf[0][0].pos = (v2i16){0, 0};
        chunk_buf[0][1].pos = (v2i16){0, -1};
        chunk_buf[0][2].pos = (v2i16){0, -2};
        chunk_buf[0][3].pos = (v2i16){-1, 0};
        chunk_buf[0][4].pos = (v2i16){0, 1};
        chunk_buf[0][5].pos = (v2i16){0, 2};
        chunk_buf[0][6].pos = (v2i16){-1, -10};
        chunk_buf[0][7].pos = (v2i16){1, 0};
        parse_chunk_states(&chunk_buf[0][0], 2);
        parse_chunk_states(&chunk_buf[0][1], 1);
        parse_chunk_states(&chunk_buf[0][2], 1);
        parse_chunk_states(&chunk_buf[0][3], 30);
        parse_chunk_states(&chunk_buf[0][4], 2);
        parse_chunk_states(&chunk_buf[0][5], 2);
        parse_chunk_states(&chunk_buf[0][6], 9);
        parse_chunk_states(&chunk_buf[0][7], 2);
    }
    lily.state |= STATE_FALLING; //temp
    lily.state &= ~STATE_PARSE_TARGET;

    if (ModeDebug)
    {
        state |= STATE_DEBUG;
        lily.state |= STATE_FLYING;
    }

    state |= STATE_ACTIVE | STATE_HUD;
}

void update_game()
{
    start_time = get_time_ms();
    win.scl = (v2f32){GetRenderWidth(), GetRenderHeight()};

    parse_player_states(&lily);
    !ModeCollide ?: give_collision_static(&lily, &target_coordinates_feet);
    give_camera_movements_player(&lily);
    if (state & STATE_DEBUG)
        give_camera_movements_debug_info(&lily);
    (lily.state & STATE_MENU_OPEN || state & STATE_SUPER_DEBUG) ? show_cursor : hide_cursor;
    update_input(&lily);

    BeginDrawing();
    ClearBackground(COL_SKYBOX); /* TODO: make actual skybox */
    BeginMode3D(lily.camera);
    { /*temp*/
        draw_chunk(&chunk_buf[0][0], 2);
        draw_chunk(&chunk_buf[0][1], 1);
        draw_chunk(&chunk_buf[0][2], 1);
        draw_chunk(&chunk_buf[0][3], 30);
        draw_chunk(&chunk_buf[0][4], 2);
        draw_chunk(&chunk_buf[0][5], 2);
        draw_chunk(&chunk_buf[0][6], 9);
        draw_chunk(&chunk_buf[0][7], 2);
    }

    if (is_range_within_v3fi(&lily.camera.target, (v3i32){-WORLD_SIZE, -WORLD_SIZE, WORLD_BOTTOM}, (v3i32){WORLD_SIZE, WORLD_SIZE, world_height}))
    {
        if (check_target_delta_position(&lily.camera.target, &lily.previous_target))
        {
            //TODO: fix segfault
            target_chunk = get_chunk(&lily.previous_target, &lily.state, STATE_PARSE_TARGET);
            printf("targetxyz[%d, %d, %d]\t\tstate[%d]\n", lily.previous_target.x, lily.previous_target.y, lily.previous_target.z, target_chunk->i[lily.previous_target.z - WORLD_BOTTOM][lily.previous_target.y][lily.previous_target.x]); /*temp*/
        }

        if (target_chunk != NULL && lily.state & STATE_PARSE_TARGET)
            if (target_chunk->i[lily.previous_target.z - WORLD_BOTTOM][lily.previous_target.y][lily.previous_target.x] & NOT_EMPTY)
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
        if (state & STATE_DEBUG)
            draw_debug_info();
        if (state_menu_depth)
        {
            if (lily.container_state & CONTR_INVENTORY)
                draw_inventory();
        }
    }

    if (state & STATE_SUPER_DEBUG)
        draw_super_debugger();
    EndDrawing();

    if (state & STATE_PAUSED)
    {
        BeginDrawing();
        draw_menu_overlay;
        EndDrawing();

        while (state & STATE_PAUSED && state & STATE_ACTIVE)
        {
            BeginDrawing();
            update_game_menu(&lily);
            draw_game_menu();
            EndDrawing();
        }
        lily.state &= ~STATE_MENU_OPEN;
    }
}

void close_game()
{
    unload_textures();
    free_gui();
    free_super_debugger();
    CloseWindow();
}

int main(void)
{
    init_game();
    while (state & STATE_ACTIVE) update_game();
    close_game();
    return 0;
}

// =============================================================================
// _section_listeners_&_input ==================================================
// =============================================================================

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
            target_chunk->i[lily.previous_target.z - WORLD_BOTTOM][lily.previous_target.y][lily.previous_target.x] = 0;
            parse_block_state(target_chunk, lily.previous_target.x, lily.previous_target.y, floorf(lily.previous_target.z - WORLD_BOTTOM));
        }
        /*old
        //TODO: refine (make correct targeting)
        u32 block_index = convert_coordinates_to_chunk_index(&lily.previous_target);
        chunk_buf[target_chunk].index[block_index] = 0;
        //parse_chunk_states(&chunk_buf[target_chunk]); //temp
        calculate_surrounding_block_state(&chunk_buf[target_chunk], block_index);
        */
    }

    if (IsMouseButtonDown(BIND_PICK_BLOCK))
    {
    }

    if (IsMouseButtonDown(BIND_USE_ITEM_OR_PLACE_BLOCK))
    {
        /*old
        //TODO: refine (make correct targeting)
        u32 block_index = convert_coordinates_to_chunk_index(&lily.previous_target);
        chunk_buf[target_chunk].index[block_index] |= NOT_EMPTY;
        //parse_chunk_states(&chunk_buf[target_chunk]); //temp
        calculate_surrounding_block_state(&chunk_buf[target_chunk], block_index);
        */
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
        if (player->container_state & CONTR_INVENTORY && player->state & STATE_MENU_OPEN)
        {
            player->container_state &= ~CONTR_INVENTORY;
            state_menu_depth = 0;
            player->state &= ~STATE_MENU_OPEN;
        }
        else if (!(player->container_state & CONTR_INVENTORY) && !(state & STATE_MENU_OPEN))
        {
            player->container_state |= CONTR_INVENTORY;
            player->state |= STATE_MENU_OPEN;
            state_menu_depth = 1;
        }

        if (!(player->container_state & CONTR_INVENTORY) && state_menu_depth)
            --state_menu_depth;
    }

    // ---- miscellaneous ------------------------------------------------------
    if (IsKeyPressed(BIND_TOGGLE_HUD))
        state ^= STATE_HUD;

    if (IsKeyPressed(BIND_TOGGLE_DEBUG))
        state ^= STATE_DEBUG;

    //TODO: fix fullscreen
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
            player->container_state = 0;
            state |= STATE_PAUSED;
            state_menu_depth = 1;
            player->state |= STATE_MENU_OPEN;
            show_cursor;
        }
        else if (state_menu_depth == 1)
        {
            player->container_state = 0;
            player->state &= ~STATE_MENU_OPEN;
            state_menu_depth = 0;
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

void update_game_menu(player *player)
{
    if (IsKeyPressed(BIND_PAUSE) && state_menu_depth == 1)
    {
        state ^= STATE_PAUSED;
        player->state &= ~STATE_MENU_OPEN;
        state_menu_depth = 0;

        buttons[BTN_BACK_TO_GAME] = BTN_INACTIVE;
        buttons[BTN_OPTIONS] = BTN_INACTIVE;
        buttons[BTN_SAVE_AND_QUIT_TO_TITLE] = BTN_INACTIVE;
    }
    if (IsKeyPressed(BIND_QUIT) && state_menu_depth)
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
