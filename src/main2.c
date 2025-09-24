#include "engine/core.c"
#include "h/main.h"

/* ---- declarations -------------------------------------------------------- */
// pthread_t thrd_chunk_handler;

/* ---- signatures ---------------------------------------------------------- */
void update_world();
void *chunk_handler();
void update_input(Player *player);
void draw_world();
void draw_gui();

int main(void)
{
    /* ---- init main ------------------------------------------------------- */
    init_textures();
    init_super_debugger(setting.render_size);

section_menu_title: /* ------------------------------------------------------ */
    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        mouse_delta = GetMouseDelta();
        if (!(state & FLAG_PARSE_CURSOR)) /* for fullscreen cursor jump prevention */
        {
            state |= FLAG_PARSE_CURSOR;
            mouse_delta = (Vector2){0.0f, 0.0f};
        }

        update_input(&lily);
        update_render_settings(setting.render_size);
        setting.render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};

        BeginDrawing();
        {
            //draw_texture_tiled(); /* TODO: draw tiled texture of title screen */
            ClearBackground(DARKBROWN); /* TODO: make actual panoramic scene */
            update_menus(setting.render_size);
        }
        EndDrawing();

        if (!(state & FLAG_PAUSED) && (state & FLAG_WORLD_LOADED))
            goto section_main;
    }

section_menu_world: /* ------------------------------------------------------ */
                    /* TODO: make real pausing instead of using the uncleared bg as still */
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

section_main: /* ------------------------------------------------------------ */
    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        mouse_delta = GetMouseDelta();
        if (!state_menu_depth && !(state & FLAG_SUPER_DEBUG))
        {
            disable_cursor;
            center_cursor;
        }
//         update_input(&lily);
        update_render_settings(setting.render_size);
        setting.render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};
//         update_world();
//
//         BeginDrawing();
//         {
//             BeginMode3D(lily.camera);
//             draw_skybox();
//            draw_chunk_tab(&block[dirt].texture);
//            draw_world();
//             EndMode3D();
//         }
//        draw_gui();
//         EndDrawing();
    }

    /* ---- section_cleanup ------------------------------------------------- */
    //pthread_join(thrd_chunk_handler, NULL); /*temp off*/
    unload_textures();
    free_chunking();
    free_gui();
    free_super_debugger();
    CloseWindow();

    return 0;
}

void init_world(str *str)
{
//     init_world_directory(str);
    if (init_chunking() != 0)
        state &= ~FLAG_ACTIVE;
//     update_player(&lily);
    update_chunk_tab(lily.chunk);
//     set_player_block(&lily, 0, 0, 0);
    lily.delta_target =
        (v3i32){
            lily.camera.target.x,
            lily.camera.target.y,
            lily.camera.target.z};
//     state |= (FLAG_HUD | FLAG_WORLD_LOADED);
}

void update_world()
{
//     if (game_tick >= SETTING_DAY_TICKS_MAX) ++game_days;
    if (state_menu_depth || (state & FLAG_SUPER_DEBUG))
        show_cursor;
    else disable_cursor;
//     update_player(&lily);
    update_player_target(&lily.camera.target, &lily.delta_target);
//     update_camera_movements_player(&lily);
//     if (MODE_COLLIDE)
//         update_collision_static(&lily);

    chunk_tab_index = get_target_chunk_index(lily.chunk, lily.delta_target);
    (chunk_tab_index >= CHUNK_BUF_VOLUME)
        ? chunk_tab_index = CHUNK_TAB_CENTER : 0;

    if (state & FLAG_CHUNK_BUF_DIRTY)
    {
        shift_chunk_tab(lily.chunk, &lily.delta_chunk);
        update_chunk_tab(lily.delta_chunk);
        state &= ~FLAG_CHUNK_BUF_DIRTY;
    }

    /* ---- player targeting ------------------------------------------------ */
    if (is_range_within_v3i(lily.delta_target,
                (v3i32){-WORLD_DIAMETER, -WORLD_DIAMETER, -WORLD_DIAMETER_VERTICAL},
                (v3i32){WORLD_DIAMETER, WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL}))
        state |= FLAG_PARSE_TARGET;
    else state &= ~FLAG_PARSE_TARGET;

    /* TODO: make a function 'index_to_bounding_box()' */
    //if (GetRayCollisionBox(GetScreenToWorldRay(cursor, lily.camera), (BoundingBox){&lily.previous_target}).hit) {}
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
//    player->movement_step_length = sqrt(pow(player->v.x, 2) + pow(player->v.y, 2));
//    if (player->movement_step_length > 0.0f)
//    {
//        player->v.x /= player->movement_step_length;
//        player->v.y /= player->movement_step_length;
//
//        player->pos.x += player->v.x * player->movement_speed;
//        player->pos.y += player->v.y * player->movement_speed;
//    }
//    /* ---- misc ------------------------------------------------------- */

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

    /* ---- debug ----------------------------------------------------------- */
#if !RELEASE_BUILD
    if (IsKeyPressed(KEY_TAB))
        state ^= FLAG_SUPER_DEBUG;
#endif /* RELEASE_BUILD */
}

void draw_world()
{
    /* ---- player chunk bounding box --------------------------------------- */
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

    /* ---- player target bounding box -------------------------------------- */
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
        draw_bounding_box_clamped(lily.pos, lily.scl, COL_Z); /*temp AABB collision*/
    }
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

