#include "engine/core.c"
#include "h/main.h"

/* ---- declarations -------------------------------------------------------- */
// pthread_t thrd_chunk_handler;

/* ---- signatures ---------------------------------------------------------- */
void *chunk_handler();
void update_input(Player *player);
void draw_world();
void draw_gui();

int
main(void)
{
    /* ---- init main ------------------------------------------------------- */
    init_textures();
    init_super_debugger(setting.render_size);

section_menu_title: /* ------------------------------------------------------ */
    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        mouse_delta = GetMouseDelta();

        /* for fullscreen cursor jump prevention */
        if (!(state & FLAG_PARSE_CURSOR))
        {
            state |= FLAG_PARSE_CURSOR;
            mouse_delta = (Vector2){0.0f, 0.0f};
        }

        update_input(&lily);
        update_render_settings(setting.render_size);
        setting.render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};

        BeginDrawing();
        {
            /* TODO: draw tiled texture of title screen */
            //draw_texture_tiled();

            /* TODO: make actual panoramic scene */
            ClearBackground(DARKBROWN);
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

struct /* Chunk Handler Args */
{
    v3i16 player_delta_chunk;
    u8 lock;
} chunk_handler_args;
void *
chunk_handler()
{
    chunk_handler_args.player_delta_chunk = lily.delta_chunk;
    chunk_handler_args.lock = 1;
    while (!WindowShouldClose() && (state & FLAG_ACTIVE))
    {
        LOGINFO("Delta[%03d %03d]",
                chunk_handler_args.player_delta_chunk.x,
                chunk_handler_args.player_delta_chunk.y);
        if (!(state & FLAG_CHUNK_BUF_DIRTY)) continue;

        chunk_handler_args.lock = 0;
        shift_chunk_tab(lily.chunk, &chunk_handler_args.player_delta_chunk);
        state &= ~FLAG_CHUNK_BUF_DIRTY;
    }
    return NULL;
}

void
update_input(Player *player)
{
//    player->movement_step_length =
//    sqrt(pow(player->v.x, 2) + pow(player->v.y, 2));
//
//    if (player->movement_step_length > 0.0f)
//    {
//        player->v.x /= player->movement_step_length;
//        player->v.y /= player->movement_step_length;
//
//        player->pos.x += player->v.x * player->movement_speed;
//        player->pos.y += player->v.y * player->movement_speed;
//    }

    /* ---- misc ------------------------------------------------------------ */

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

void
draw_world()
{
    /* ---- player chunk bounding box --------------------------------------- */
    if (state & FLAG_DEBUG_MORE)
        draw_bounding_box(
                (Vector3){
                (f32)(lily.chunk.x * CHUNK_DIAMETER) +
                ((f32)CHUNK_DIAMETER / 2),
                (f32)(lily.chunk.y * CHUNK_DIAMETER) +
                ((f32)CHUNK_DIAMETER / 2),
                (f32)(lily.chunk.z * CHUNK_DIAMETER) +
                ((f32)CHUNK_DIAMETER / 2)},

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
                [lily.delta_target.z - (chunk_tab[chunk_tab_index]->pos.z *
                    CHUNK_DIAMETER)]
                [lily.delta_target.y - (chunk_tab[chunk_tab_index]->pos.y *
                    CHUNK_DIAMETER)]
                [lily.delta_target.x - (chunk_tab[chunk_tab_index]->pos.x *
                    CHUNK_DIAMETER)] & NOT_EMPTY)
        {
            draw_block_wires(lily.delta_target);
            if (state & FLAG_DEBUG_MORE)
                DrawLine3D(Vector3Subtract(lily.camera.position,
                            (Vector3){0.0f, 0.0f, 0.5f}),
                        lily.camera.target, RED);
        }
        else if (state & FLAG_DEBUG_MORE)
            DrawLine3D(Vector3Subtract(lily.camera.position,
                        (Vector3){0.0f, 0.0f, 0.5f}),
                    lily.camera.target, GREEN);
    }

    if (state & FLAG_DEBUG_MORE)
    {
//        draw_block_wires(&target_coordinates_feet);
//        printf("feet: %d %d %d\n",
//                target_coordinates_feet.x,
//                target_coordinates_feet.y,
//                target_coordinates_feet.z);

        DrawCubeWiresV(lily.camera.target, (Vector3){1.0f, 1.0f, 1.0f}, GREEN);
        draw_bounding_box(lily.pos, lily.scl, RAYWHITE);

        /*temp AABB collision*/
        draw_bounding_box_clamped(lily.pos, lily.scl, COL_Z);
    }
}

void
draw_gui()
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
