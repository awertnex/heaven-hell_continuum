#include <string.h>
#include <sys/stat.h>
#include <math.h>

#include "dependencies/raylib-5.5/src/raylib.h"
#include "dependencies/raylib-5.5/src/rlgl.h"

#if defined __linux__
    #include "linux_minecraft.c"
#elif defined _WIN32 || defined _WIN64 || defined __CYGWIN__s
    #include "windows_minecraft.c"
#endif // PLATFORM

// ---- variables --------------------------------------------------------------
v2f32 render_size = {WIDTH, HEIGHT};
u16 state = 0;
u8 state_menu_depth = 0;
f64 game_start_time = 0.0f;
u64 game_tick = 0;
u64 game_days = 0;
settings setting =
{
    .reach_distance =       SETTING_REACH_DISTANCE_MAX,
    .fov =                  SETTING_FOV_DEFAULT,
    .mouse_sensitivity =    SETTING_MOUSE_SENSITIVITY_DEFAULT * 0.065f,
    .render_distance =      SETTING_RENDER_DISTANCE_DEFAULT,
    .gui_scale =            SETTING_GUI_SCALE_DEFAULT,
};
Player lily =
{
    .name = "Lily",
    .pos = {0.0f},
    .scl = {0.6f, 0.6f, 1.8f},
    .collision_check_start = {0.0f},
    .collision_check_end = {0.0f},
    .pitch = -29.0f,
    .yaw = 121.0f,
    .sin_pitch = 0.0f, .cos_pitch = 0.0f, .sin_yaw = 0.0f, .cos_yaw = 0.0f,
    .eye_height = 1.5f,
    .m = 2.0f,
    .movement_speed = 10.0f,
    .container_state = 0,
    .perspective = 0,

    .camera =
    {
        .up.z = 1.0f,
        .fovy = 70.0f,
        .projection = CAMERA_PERSPECTIVE,
    },
    .camera_distance = SETTING_CAMERA_DISTANCE_MAX,
    .camera_debug_info =
    {
        .up.z = 1.0f,
        .fovy = 50.0f,
        .projection = CAMERA_ORTHOGRAPHIC,
    },

    .spawn_point = {0},
};

// ---- signatures -------------------------------------------------------------
void update_world();
void update_input(Player *player);
void draw_skybox();

int main(int argc, char **argv)
{
    // ---- game init ----------------------------------------------------------
    if (MODE_DEBUG)
        LOGINFO("%s", "Debugging Enabled");

    init_paths();
    //TODO: load textures
    //init_texture_layouts();
    //init_textures();

    if (argc > 1)
    {
        if (strncmp(argv[1], "instance ", 9) && argv[2][0])
        {
            for (u8 i = 2; i < argc; ++i)
            {
                init_instance_directory(argv[i], &state, STATE_ACTIVE);
                if (!(state & STATE_ACTIVE))
                    return -1;
            }
        }
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WIDTH, HEIGHT, "minecraft.c");
    SetExitKey(KEY_PAUSE);
    SetWindowMinSize(640, 480);
#if !RELEASE_BUILD
    SetTargetFPS(60); // TODO: make release-build FPS depend on video settings
#endif // RELEASE_BUILD

    init_fonts();
    init_gui();
    apply_render_settings();
    update_render_settings(render_size);
    init_super_debugger(render_size);

    setting.render_distance = SETTING_RENDER_DISTANCE_MAX; //temp
    game_start_time = get_time_ms();

    state |= STATE_ACTIVE;
    while (!WindowShouldClose() && (state & STATE_ACTIVE))
    {
        // ---- game loop ------------------------------------------------------
        mouse_delta = GetMouseDelta();
        if (!state_menu_depth && !(state & STATE_SUPER_DEBUG))
        {
            disable_cursor;
            center_cursor;
        }

        update_input(&lily);
        update_render_settings(render_size);
        render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};
        BeginDrawing();
        //draw_texture_tiled(); // TODO: draw tiled texture of title screen
        ClearBackground(DARKBROWN); // TODO: make actual panoramic scene
        update_menus(render_size);
        if (state & STATE_WORLD_LOADED)
        {
            draw_skybox();
            update_world();
        }
        EndDrawing();

        if (state & STATE_PAUSED) // TODO: make real pausing instead of using the uncleared bg as still
        {
            BeginDrawing();
            draw_menu_overlay(render_size);
            EndDrawing();

            while ((state & STATE_PAUSED) && (state & STATE_ACTIVE) && !WindowShouldClose())
            {
                render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};
                update_input(&lily);
                BeginDrawing();
                update_menus(render_size);
                EndDrawing();
            }
        }
    }

    // ---- game close ---------------------------------------------------------
    UnloadTexture(tex_cobblestone); //temp
    UnloadTexture(tex_dirt); //temp
    unload_textures();
    free_chunking();
    free_gui();
    free_super_debugger();
    CloseWindow();
    return 0;
}

void init_world(const char *str)
{
    rlSetClipPlanes(0.1f, 500.0f);

    if (init_chunking() != 0)
        state &= ~STATE_ACTIVE;

    { //temp
        /* temp off
        chunk_buf[GET_CHUNK_XY(0, 0)].pos = (v2i16){0, 0};
        chunk_buf[GET_CHUNK_XY(1, 0)].pos = (v2i16){0, -1};
        chunk_buf[GET_CHUNK_XY(2, 0)].pos = (v2i16){0, -2};
        chunk_buf[GET_CHUNK_XY(3, 0)].pos = (v2i16){-1, 0};
        chunk_buf[GET_CHUNK_XY(4, 0)].pos = (v2i16){0, 1};
        chunk_buf[GET_CHUNK_XY(5, 0)].pos = (v2i16){0, 2};
        chunk_buf[GET_CHUNK_XY(6, 0)].pos = (v2i16){-1, -1};
        chunk_buf[GET_CHUNK_XY(7, 0)].pos = (v2i16){1, 0};
        load_chunk(&chunk_buf[GET_CHUNK_XY(0, 0)]);
        load_chunk(&chunk_buf[GET_CHUNK_XY(1, 0)]);
        load_chunk(&chunk_buf[GET_CHUNK_XY(2, 0)]);
        load_chunk(&chunk_buf[GET_CHUNK_XY(3, 0)]);
        load_chunk(&chunk_buf[GET_CHUNK_XY(4, 0)]);
        load_chunk(&chunk_buf[GET_CHUNK_XY(5, 0)]);
        load_chunk(&chunk_buf[GET_CHUNK_XY(6, 0)]);
        load_chunk(&chunk_buf[GET_CHUNK_XY(7, 0)]);
        */

        tex_cobblestone =   LoadTexture("resources/textures/blocks/stone.png");
        tex_dirt =          LoadTexture("resources/textures/blocks/dirt.png");

        lily.state |= STATE_FALLING;
    }

    lily.state &= ~STATE_PARSE_TARGET;
    set_player_block(&lily, 0, 0, 0);

    state |= (STATE_HUD | STATE_WORLD_LOADED);
    LOGINFO("%s '%s'", "World Loaded", "Poop Consistency Tester");

    disable_cursor;
    center_cursor;
}

void update_world()
{
    game_tick = (floor((get_time_ms() - game_start_time) * 20)) - (SETTING_DAY_TICKS_MAX * game_days);
    if (game_tick >= SETTING_DAY_TICKS_MAX)
        ++game_days;

    if (MODE_COLLIDE)
        update_collision_static(&lily);

    if (state_menu_depth || (state & STATE_SUPER_DEBUG))
        show_cursor;
    else disable_cursor;

    if (state & STATE_DEBUG)
        update_camera_movements_debug_info(&lily.camera_debug_info, &lily);

    update_player_states(&lily);
    update_camera_movements_player(&lily);

    if (lily.state & STATE_CHUNK_BUF_DIRTY)
    {
        update_chunk_buf(&lily.chunk);
        lily.state &= ~STATE_CHUNK_BUF_DIRTY;
    }

    BeginMode3D(lily.camera);
    draw_chunk_buf();

    //TODO: make a function 'index_to_bounding_box()'
    //if (GetRayCollisionBox(GetScreenToWorldRay(cursor, lily.camera), (BoundingBox){&lily.previous_target}).hit)
    //{}
    if (is_range_within_v3fi(&lily.camera.target,
                (v3i32){-WORLD_DIAMETER, -WORLD_DIAMETER, WORLD_BOTTOM},
                (v3i32){WORLD_DIAMETER, WORLD_DIAMETER, world_height}))
    {
        if (state & STATE_DEBUG_MORE)
            draw_bounding_box(
                    (Vector3){lily.chunk.x + ((f32)CHUNK_DIAMETER / 2), lily.chunk.y + ((f32)CHUNK_DIAMETER / 2), WORLD_BOTTOM},
                    (Vector3){CHUNK_DIAMETER, CHUNK_DIAMETER, world_height});

        if (check_delta_target(&lily.camera.target, &lily.delta_target))
            target_chunk = get_chunk(&lily.delta_target, &lily.state, STATE_PARSE_TARGET);

        if (target_chunk != NULL && lily.state & STATE_PARSE_TARGET && (state & STATE_HUD))
        {
            if (target_chunk->i
                    [lily.delta_target.z - WORLD_BOTTOM]
                    [lily.delta_target.y - (target_chunk->pos.y * CHUNK_DIAMETER)]
                    [lily.delta_target.x - (target_chunk->pos.x * CHUNK_DIAMETER)] & NOT_EMPTY)
            {
                draw_block_wires(lily.delta_target);
                if (state & STATE_DEBUG_MORE)
                    DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}), lily.camera.target, RED);
            } else if (state & STATE_DEBUG_MORE)
                DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}), lily.camera.target, GREEN);
        }
    }

    if (state & STATE_DEBUG_MORE)
    {
        /*temp
          draw_block_wires(&target_coordinates_feet);
          printf("feet: %d %d %d\n", target_coordinates_feet.x, target_coordinates_feet.y, target_coordinates_feet.z);
          */
        DrawCubeWiresV(lily.camera.target, (Vector3){1.0f, 1.0f, 1.0f}, GREEN);
        draw_bounding_box(lily.pos, lily.scl);
        draw_bounding_box_clamped(lily.pos, lily.scl); //temp AABB collision
        draw_default_grid(COL_X, COL_Y, COL_Z);
    }

    EndMode3D();

    if (state & STATE_HUD)
    {
        draw_hud();
        draw_debug_info(&lily.camera_debug_info);
        if (state_menu_depth && lily.container_state)
            draw_containers(&lily, render_size);
    }

    draw_super_debugger(render_size);
}

void update_input(Player *player)
{
    // ---- jumping ------------------------------------------------------------
    if (IsKeyDown(bind_jump))
    {
        if (IsKeyPressed(bind_jump) && get_double_press(player, bind_jump))
            player->state ^= STATE_FLYING;

        if (player->state & STATE_FLYING)
            player->pos.z += player->movement_speed;

        if (player->state & STATE_CAN_JUMP)
        {
            player->v.z += PLAYER_JUMP_HEIGHT;
            player->state &= ~STATE_CAN_JUMP;
        }
    }

    // ---- sneaking -----------------------------------------------------------
    if (IsKeyDown(bind_sneak))
    {
        if (player->state & STATE_FLYING)
            player->pos.z -= player->movement_speed;
        else player->state |= STATE_SNEAKING;
    }

    if (IsKeyUp(bind_sneak))
        player->state &= ~STATE_SNEAKING;

    // ---- sprinting ----------------------------------------------------------
    if (IsKeyDown(bind_sprint) && IsKeyDown(bind_walk_forwards))
        player->state |= STATE_SPRINTING;

    if (IsKeyUp(bind_sprint) && IsKeyUp(bind_walk_forwards))
        player->state &= ~STATE_SPRINTING;

    // ---- moving -------------------------------------------------------------
    if (IsKeyDown(bind_strafe_left))
    {
        player->v.x -= player->movement_speed * player->sin_yaw;
        player->v.y += player->movement_speed * player->cos_yaw;
    }

    if (IsKeyDown(bind_strafe_right))
    {
        player->v.x += player->movement_speed * player->sin_yaw;
        player->v.y -= player->movement_speed * player->cos_yaw;
    }

    if (IsKeyDown(bind_walk_backwards))
    {
        player->v.x -= player->movement_speed * player->cos_yaw;
        player->v.y -= player->movement_speed * player->sin_yaw;
    }

    if (IsKeyDown(bind_walk_forwards))
    {
        if (IsKeyPressed(bind_walk_forwards) && get_double_press(player, bind_walk_forwards))
            player->state |= STATE_SPRINTING;

        player->v.x += player->movement_speed * player->cos_yaw;
        player->v.y += player->movement_speed * player->sin_yaw;
    }

    player->movement_step_length = sqrt(pow(player->v.x, 2) + pow(player->v.y, 2));
    if (player->movement_step_length > 0.0f)
    {
        player->v.x /= player->movement_step_length;
        player->v.y /= player->movement_step_length;

        player->pos.x += player->v.x * player->movement_speed;
        player->pos.y += player->v.y * player->movement_speed;
    }

    // ---- gameplay -----------------------------------------------------------
    if (IsMouseButtonDown(bind_attack_or_destroy))
    {
        if (player->state & STATE_PARSE_TARGET)
            remove_block(target_chunk, lily.delta_target.x, lily.delta_target.y, floorf(lily.delta_target.z - WORLD_BOTTOM));
    }

    if (IsMouseButtonDown(bind_pick_block))
    {
    }

    if (IsMouseButtonDown(bind_use_item_or_place_block))
    {
        if (player->state & STATE_PARSE_TARGET)
            add_block(target_chunk, lily.delta_target.x, lily.delta_target.y, floorf(lily.delta_target.z - WORLD_BOTTOM));
    }

    // ---- inventory ----------------------------------------------------------
    if (IsKeyPressed(bind_hotbar_slot_1) || IsKeyPressed(bind_hotbar_slot_kp_1))
        hotbar_slot_selected = 1;

    if (IsKeyPressed(bind_hotbar_slot_2) || IsKeyPressed(bind_hotbar_slot_kp_2))
        hotbar_slot_selected = 2;

    if (IsKeyPressed(bind_hotbar_slot_3) || IsKeyPressed(bind_hotbar_slot_kp_3))
        hotbar_slot_selected = 3;

    if (IsKeyPressed(bind_hotbar_slot_4) || IsKeyPressed(bind_hotbar_slot_kp_4))
        hotbar_slot_selected = 4;

    if (IsKeyPressed(bind_hotbar_slot_5) || IsKeyPressed(bind_hotbar_slot_kp_5))
        hotbar_slot_selected = 5;

    if (IsKeyPressed(bind_hotbar_slot_6) || IsKeyPressed(bind_hotbar_slot_kp_6))
        hotbar_slot_selected = 6;

    if (IsKeyPressed(bind_hotbar_slot_7) || IsKeyPressed(bind_hotbar_slot_kp_7))
        hotbar_slot_selected = 7;

    if (IsKeyPressed(bind_hotbar_slot_8) || IsKeyPressed(bind_hotbar_slot_kp_8))
        hotbar_slot_selected = 8;

    if (IsKeyPressed(bind_hotbar_slot_9) || IsKeyPressed(bind_hotbar_slot_kp_9))
        hotbar_slot_selected = 9;

    if (IsKeyPressed(bind_hotbar_slot_0) || IsKeyPressed(bind_hotbar_slot_kp_0))
        hotbar_slot_selected = 10;

    if (IsKeyPressed(bind_open_or_close_inventory))
    {
        if (player->container_state & CONTR_INVENTORY && state_menu_depth)
        {
            state_menu_depth = 0;
            player->container_state &= ~CONTR_INVENTORY;
        } else if (!(player->container_state & CONTR_INVENTORY) && !state_menu_depth)
        {
            state_menu_depth = 1;
            player->container_state |= CONTR_INVENTORY;
        }

        if (!(player->container_state & CONTR_INVENTORY) && state_menu_depth)
            --state_menu_depth;
    }

    // ---- miscellaneous ------------------------------------------------------
    if (IsKeyPressed(bind_toggle_hud))
        state ^= STATE_HUD;

    if (IsKeyPressed(bind_toggle_debug))
    {
        if (state & STATE_DEBUG)
            state &= ~(STATE_DEBUG | STATE_DEBUG_MORE);
        else state |= STATE_DEBUG;

        if (IsKeyDown(KEY_LEFT_SHIFT) && (state & STATE_DEBUG))
            state |= STATE_DEBUG_MORE;
    }

    if (IsKeyPressed(bind_toggle_fullscreen))
    {
        state ^= STATE_FULLSCREEN;

        if (state & STATE_FULLSCREEN)
        {
            ToggleBorderlessWindowed();
            SetConfigFlags(FLAG_FULLSCREEN_MODE);
        }
        else
        {
            SetConfigFlags(~FLAG_FULLSCREEN_MODE);
            ToggleBorderlessWindowed();
        }
    }

    if (IsKeyPressed(bind_toggle_perspective))
    {
        if (player->perspective < 4)
            ++player->perspective;
        else player->perspective = 0;
    }

    if (IsKeyPressed(bind_pause) && (state & STATE_WORLD_LOADED))
    {
        if (state & STATE_WORLD_LOADED)
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
                btn_func_back_to_game();
        }
        else
        {
            if (state_menu_depth == 1)
                state &= ~STATE_ACTIVE;
            else btn_func_back();
        }
    }

    // ---- debug --------------------------------------------------------------
#if (RELEASE_BUILD == 0)
    if (IsKeyPressed(KEY_TAB))
        state ^= STATE_SUPER_DEBUG;
#endif // RELEASE_BUILD
}

f64 skybox_time = 0;
f64 skybox_mid_day = 0;
f64 skybox_pre_burn = 0;
f64 skybox_burn = 0;
f64 skybox_burn_boost = 0;
f64 skybox_mid_night = 0;
Color skybox_rgba = {0};
void draw_skybox()
{
    skybox_time =        (f64)game_tick / SETTING_DAY_TICKS_MAX;
    skybox_mid_day =      fabs(sinf(1.5f * powf(sinf(skybox_time * PI), 1.0f)));
    skybox_pre_burn =     fabs(sinf(powf(sinf((skybox_time + 0.33) * PI * 1.2f), 16.0f)));
    skybox_burn =        fabs(sinf(1.5f * powf(sinf((skybox_time + 0.124f) * PI * 1.6f), 20.0f)));
    skybox_burn_boost =   fabs(powf(sinf((skybox_time + 0.212f) * PI * 1.4f), 64.0f));
    skybox_mid_night =    fabs(sinf(powf(2 * cosf(skybox_time * PI), 3.0f)));
    skybox_rgba = (Color){
            Clamp((skybox_mid_day * 171) + (skybox_burn * 85) + (skybox_mid_night * 1) +     (skybox_pre_burn * 13) +  (skybox_burn_boost * 76), 0, 255),
            Clamp((skybox_mid_day * 229) + (skybox_burn * 42) + (skybox_mid_night * 4) +     (skybox_pre_burn * 7) +   (skybox_burn_boost * 34), 0, 255),
            Clamp((skybox_mid_day * 255) + (skybox_burn * 19) + (skybox_mid_night * 14) +    (skybox_pre_burn * 20),                           0, 255),
            255
        };

    ClearBackground(skybox_rgba);
}

