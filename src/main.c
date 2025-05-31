#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <pthread.h>

#include "dependencies/raylib-5.5/include/raylib.h"
#include "dependencies/raylib-5.5/include/rlgl.h"

#if defined __linux__
    #include "linux_minecraft.c"
#elif defined _WIN32 || defined _WIN64 || defined __CYGWIN__s
    #include "windows_minecraft.c"
#endif // PLATFORM

// ---- declarations -----------------------------------------------------------
u16 state = 0;
u8 state_menu_depth = 0;
f64 game_start_time = 0.0f;
u64 game_tick = 0;
u64 game_days = 0;
pthread_t thrd_chunk_handler;

Settings setting =
{
    .render_size =          (v2f32){854, 480},
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
void *chunk_handler();
void update_input(Player *player);
void draw_skybox();
void draw_world();
void draw_gui();

int main(void)
{
    // ---- init main ----------------------------------------------------------
    if (MODE_DEBUG)
        LOGINFO("%s", "Debugging Enabled");

    state = FLAG_ACTIVE | FLAG_PARSE_CURSOR | FLAG_PAUSED;
    init_paths();
#if RELEASE_BUILD
    init_instance_directory("new_instance"); // TODO: make editable instance name
#else
    init_instance_directory("test_instance");
#endif // RELEASE_BUILD

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(setting.render_size.x, setting.render_size.y, "minecraft.c");
    SetExitKey(KEY_PAUSE);
    SetWindowMinSize(640, 480);

    init_textures();

#if !RELEASE_BUILD
    SetTargetFPS(60); // TODO: make release-build FPS depend on video settings
#endif // RELEASE_BUILD

    init_fonts();
    init_gui();
    apply_render_settings();
    update_render_settings(setting.render_size);
    init_super_debugger(setting.render_size);

    game_start_time = get_time_ms();

section_menu_title: // ---------------------------------------------------------
    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        mouse_delta = GetMouseDelta();
        if (!(state & FLAG_PARSE_CURSOR)) // for fullscreen cursor jump prevention
        {
            state |= FLAG_PARSE_CURSOR;
            mouse_delta = (Vector2){0.0f, 0.0f};
        }

        update_input(&lily);
        update_render_settings(setting.render_size);
        setting.render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};

        BeginDrawing();
        {
            //draw_texture_tiled(); // TODO: draw tiled texture of title screen
            ClearBackground(DARKBROWN); // TODO: make actual panoramic scene
            update_menus(setting.render_size);
        }
        EndDrawing();

        if (!(state & FLAG_PAUSED) && (state & FLAG_WORLD_LOADED))
            goto section_main;
    }

section_menu_world: // ---------------------------------------------------------
                    // TODO: make real pausing instead of using the uncleared bg as still
    BeginDrawing();
    draw_menu_overlay(setting.render_size);
    EndDrawing();

    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        update_input(&lily);
        setting.render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};
        update_render_settings(setting.render_size);

        BeginDrawing();
        update_menus(setting.render_size);
        EndDrawing();

        if (!(state & FLAG_WORLD_LOADED))
            goto section_menu_title;

        if (!(state & FLAG_PAUSED) && (state & FLAG_WORLD_LOADED))
            break;
    }

section_main: // ---------------------------------------------------------------
    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        mouse_delta = GetMouseDelta();
        if (!state_menu_depth && !(state & FLAG_SUPER_DEBUG))
        {
            disable_cursor;
            center_cursor;
        }

        if (!(state & FLAG_PARSE_CURSOR)) // for fullscreen cursor jump prevention
        {
            state |= FLAG_PARSE_CURSOR;
            mouse_delta = (Vector2){0.0f, 0.0f};
        }

        update_input(&lily);
        update_render_settings(setting.render_size);
        setting.render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};
        update_world();

        BeginDrawing();
        {
            BeginMode3D(lily.camera);
            draw_skybox();
            draw_chunk_tab(&block[dirt].texture);
            draw_world();
            EndMode3D();
        }
        draw_gui();
        EndDrawing();

        if (!(state & FLAG_WORLD_LOADED))
            goto section_menu_title;

        if (state & FLAG_PAUSED)
            goto section_menu_world;
    }

    // ---- section_cleanup ----------------------------------------------------
    //pthread_join(thrd_chunk_handler, NULL); //temp off
    unload_textures();
    free_chunking();
    free_gui();
    free_super_debugger();
    CloseWindow();

    return 0;
}

void init_world(str *str)
{
    if (strlen(str) == 0) return;

    init_world_directory(str);
    rlSetClipPlanes(0.1f, 500.0f);
    if (init_chunking() != 0)
        state &= ~FLAG_ACTIVE;

    update_player(&lily);
    update_chunk_tab(lily.chunk);

    //lily.state |= FLAG_FALLING; //temp off
    lily.state |= FLAG_FLYING; //temp
    set_player_block(&lily, 0, 0, 0);
    lily.delta_target =
        (v3i32){
            lily.camera.target.x,
            lily.camera.target.y,
            lily.camera.target.z};

    state |= (FLAG_HUD | FLAG_WORLD_LOADED);

    disable_cursor;
    center_cursor;
}

void update_world()
{
    game_tick = (floor((get_time_ms() - game_start_time) * 20)) - (SETTING_DAY_TICKS_MAX * game_days);
    if (game_tick >= SETTING_DAY_TICKS_MAX)
        ++game_days;

    if (state_menu_depth || (state & FLAG_SUPER_DEBUG))
        show_cursor;
    else disable_cursor;

    update_player(&lily);
    update_player_target(&lily.camera.target, &lily.delta_target);
    update_camera_movements_player(&lily);
    if (MODE_COLLIDE)
        update_collision_static(&lily);

    chunk_tab_index = get_target_chunk_index(lily.chunk, lily.delta_target);
    (chunk_tab_index >= CHUNK_BUF_VOLUME)
        ? chunk_tab_index = CHUNK_TAB_CENTER : 0;

    if (state & FLAG_CHUNK_BUF_DIRTY)
    {
        shift_chunk_tab(lily.chunk, &lily.delta_chunk);
        update_chunk_tab(lily.delta_chunk);
        state &= ~FLAG_CHUNK_BUF_DIRTY;
    }

    // ---- player targeting ---------------------------------------------------
    if (is_range_within_v3i(lily.delta_target,
                (v3i32){-WORLD_DIAMETER, -WORLD_DIAMETER, -WORLD_DIAMETER_VERTICAL},
                (v3i32){WORLD_DIAMETER, WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL}))
        state |= FLAG_PARSE_TARGET;
    else state &= ~FLAG_PARSE_TARGET;

    //TODO: make a function 'index_to_bounding_box()'
    //if (GetRayCollisionBox(GetScreenToWorldRay(cursor, lily.camera), (BoundingBox){&lily.previous_target}).hit)
    //{}
}

struct /* Chunk Handler Args */
{
    v3i16 player_delta_chunk;
    u8 lock;
} chunk_handler_args;
void *chunk_handler()
{
    chunk_handler_args.player_delta_chunk = lily.delta_chunk;
    chunk_handler_args.lock = 1;
    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        LOGINFO("Delta[%03d %03d]", chunk_handler_args.player_delta_chunk.x, chunk_handler_args.player_delta_chunk.y);
        if (!(state & FLAG_CHUNK_BUF_DIRTY)) continue;

        chunk_handler_args.lock = 0;
        shift_chunk_tab(lily.chunk, &chunk_handler_args.player_delta_chunk);
        state &= ~FLAG_CHUNK_BUF_DIRTY;
    }
    return NULL;
}

void update_input(Player *player)
{
    // ---- jumping ------------------------------------------------------------
    if (IsKeyDown(bind_jump))
    {
        if (IsKeyPressed(bind_jump) && get_double_press(bind_jump))
            player->state ^= FLAG_FLYING;

        if (player->state & FLAG_FLYING)
            player->pos.z += player->movement_speed;

        if (player->state & FLAG_CAN_JUMP)
        {
            player->v.z += PLAYER_JUMP_HEIGHT;
            player->state &= ~FLAG_CAN_JUMP;
        }
    }

    // ---- sneaking -----------------------------------------------------------
    if (IsKeyDown(bind_sneak))
    {
        if (player->state & FLAG_FLYING)
            player->pos.z -= player->movement_speed;
        else player->state |= FLAG_SNEAKING;
    }
    else player->state &= ~FLAG_SNEAKING;

    // ---- sprinting ----------------------------------------------------------
    if (IsKeyDown(bind_sprint) && IsKeyDown(bind_walk_forwards))
        player->state |= FLAG_SPRINTING;

    if (IsKeyUp(bind_sprint) && IsKeyUp(bind_walk_forwards))
        player->state &= ~FLAG_SPRINTING;

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
        if (IsKeyPressed(bind_walk_forwards) && get_double_press(bind_walk_forwards))
            player->state |= FLAG_SPRINTING;

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
        if ((state & FLAG_PARSE_TARGET)
                && !(state & FLAG_CHUNK_BUF_DIRTY)
                && (chunk_tab[chunk_tab_index] != NULL))
        {
            remove_block(chunk_tab_index,
                    lily.delta_target.x,
                    lily.delta_target.y,
                    lily.delta_target.z);
        }
    }

    if (IsMouseButtonDown(bind_pick_block))
    {
    }

    if (IsMouseButtonDown(bind_use_item_or_place_block))
    {
        if ((state & FLAG_PARSE_TARGET)
                && !(state & FLAG_CHUNK_BUF_DIRTY)
                && (chunk_tab[chunk_tab_index] != NULL))
        {
            add_block(chunk_tab_index,
                    lily.delta_target.x,
                    lily.delta_target.y,
                    lily.delta_target.z);
        }
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
        if ((player->container_state & STATE_CONTR_INVENTORY) && state_menu_depth)
        {
            state_menu_depth = 0;
            player->container_state &= ~STATE_CONTR_INVENTORY;
        }
        else if (!(player->container_state & STATE_CONTR_INVENTORY) && !state_menu_depth)
        {
            state_menu_depth = 1;
            player->container_state |= STATE_CONTR_INVENTORY;
        }

        if (!(player->container_state & STATE_CONTR_INVENTORY) && state_menu_depth)
            --state_menu_depth;
    }

    // ---- miscellaneous ------------------------------------------------------
    if (IsKeyPressed(bind_toggle_hud))
        state ^= FLAG_HUD;

    if (IsKeyPressed(bind_toggle_debug))
    {
        if (state & FLAG_DEBUG)
            state &= ~(FLAG_DEBUG | FLAG_DEBUG_MORE);
        else state |= FLAG_DEBUG;

        if (IsKeyDown(KEY_LEFT_SHIFT) && (state & FLAG_DEBUG))
            state |= FLAG_DEBUG_MORE;
    }

    if (IsKeyPressed(bind_toggle_fullscreen))
    {
        state ^= FLAG_FULLSCREEN;
        state &= ~FLAG_PARSE_CURSOR;

        if (state & FLAG_FULLSCREEN)
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

    if (IsKeyPressed(bind_pause) && (state & FLAG_WORLD_LOADED))
    {
        if (state & FLAG_WORLD_LOADED)
        {
            if (!state_menu_depth)
            {
                state_menu_depth = 1;
                menu_index = MENU_GAME;
                state |= FLAG_PAUSED;
                player->container_state = 0;
                show_cursor;
            }
            else if (state_menu_depth == 1)
                btn_func_back_to_game();
        }
        else
        {
            if (state_menu_depth == 1)
                state &= ~FLAG_ACTIVE;
            else btn_func_back();
        }
    }

    // ---- debug --------------------------------------------------------------
#if !RELEASE_BUILD
    if (IsKeyPressed(KEY_TAB))
        state ^= FLAG_SUPER_DEBUG;
#endif // RELEASE_BUILD
}

void draw_world()
{
    // ---- player chunk bounding box ------------------------------------------
    if (state & FLAG_DEBUG_MORE)
        draw_bounding_box(
                (Vector3){
                (f32)(lily.chunk.x * CHUNK_DIAMETER) + ((f32)CHUNK_DIAMETER / 2),
                (f32)(lily.chunk.y * CHUNK_DIAMETER) + ((f32)CHUNK_DIAMETER / 2),
                (f32)(lily.chunk.z * CHUNK_DIAMETER) + ((f32)CHUNK_DIAMETER / 2)},
                (Vector3){
                (f32)CHUNK_DIAMETER,
                (f32)CHUNK_DIAMETER,
                (f32)CHUNK_DIAMETER},
                ORANGE);

    // ---- player target bounding box -----------------------------------------
    if ((state & FLAG_PARSE_TARGET)
            && (state & FLAG_HUD)
            && chunk_tab[chunk_tab_index] != NULL)
    {
        if (chunk_tab[chunk_tab_index]->block
                [lily.delta_target.z - (chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER)]
                [lily.delta_target.y - (chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER)]
                [lily.delta_target.x - (chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER)]
                & NOT_EMPTY)
        {
            draw_block_wires(lily.delta_target);
            if (state & FLAG_DEBUG_MORE)
                DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}),
                        lily.camera.target, RED);
        }
        else if (state & FLAG_DEBUG_MORE)
            DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}),
                    lily.camera.target, GREEN);
    }

    if (state & FLAG_DEBUG_MORE)
    {
        /*temp
          draw_block_wires(&target_coordinates_feet);
          printf("feet: %d %d %d\n", target_coordinates_feet.x, target_coordinates_feet.y, target_coordinates_feet.z);
          */
        DrawCubeWiresV(lily.camera.target, (Vector3){1.0f, 1.0f, 1.0f}, GREEN);
        draw_bounding_box(lily.pos, lily.scl, RAYWHITE);
        draw_bounding_box_clamped(lily.pos, lily.scl, COL_Z); //temp AABB collision
    }
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
    skybox_time =       (f64)game_tick / (f64)SETTING_DAY_TICKS_MAX;
    skybox_mid_day =    fabs(sin(1.5f * pow(sin(skybox_time * PI), 1.0f)));
    skybox_pre_burn =   fabs(sin(pow(sin((skybox_time + 0.33) * PI * 1.2f), 16.0f)));
    skybox_burn =       fabs(sin(1.5f * pow(sin((skybox_time + 0.124f) * PI * 1.6f), 32.0f)));
    skybox_burn_boost = fabs(pow(sin((skybox_time + 0.212f) * PI * 1.4f), 64.0f));
    skybox_mid_night =  fabs(sin(pow(2 * cos(skybox_time * PI), 3.0f)));
    skybox_rgba =
        (Color){
            Clamp((skybox_mid_day * 171) + (skybox_burn * 85) + (skybox_mid_night * 1) +
                    (skybox_pre_burn * 13) + (skybox_burn_boost * 76), 0, 255),

            Clamp((skybox_mid_day * 229) + (skybox_burn * 42) + (skybox_mid_night * 4) +
                    (skybox_pre_burn * 7) + (skybox_burn_boost * 34), 0, 255),

            Clamp((skybox_mid_day * 255) + (skybox_burn * 19) + (skybox_mid_night * 14) +
                    (skybox_pre_burn * 20), 0, 255),
            255,
        };

    ClearBackground(skybox_rgba);
}

void draw_gui()
{
    if (state & FLAG_HUD)
    {
        draw_hud();
        draw_debug_info(&lily.camera_debug_info);
    }

    if (state_menu_depth && lily.container_state)
        draw_containers(&lily, setting.render_size);

    if (state & FLAG_SUPER_DEBUG)
        draw_super_debugger(setting.render_size);
}

