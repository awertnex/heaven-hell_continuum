int main(void)
{
    init_textures();
    init_super_debugger(setting.render_size);

section_menu_title: /* ------------------------------------------------------ */
    while (!WindowShouldClose() && (flag & FLAG_ACTIVE))
    {
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

        if (!(flag & FLAG_PAUSED) && (flag & FLAG_WORLD_LOADED))
            goto section_main;
    }

section_menu_world: /* ------------------------------------------------------ */

    /* TODO: make real pausing instead of using the uncleared bg as still */
    BeginDrawing();
    draw_menu_overlay(setting.render_size);
    EndDrawing();

    while (!WindowShouldClose() && (flag & FLAG_ACTIVE))
    {
//        player->movement_step_length =
//        sqrt(pow(player->v.x, 2) + pow(player->v.y, 2));
//
//        if (player->movement_step_length > 0.0f)
//        {
//            player->v.x /= player->movement_step_length;
//            player->v.y /= player->movement_step_length;
//
//            player->pos.x += player->v.x * player->movement_speed;
//            player->pos.y += player->v.y * player->movement_speed;
//        }

        /* ---- misc -------------------------------------------------------- */
        if (IsKeyPressed(bind_toggle_fullscreen))
        {
            flag ^= FLAG_FULLSCREEN;
            flag &= ~FLAG_PARSE_CURSOR;

            if (flag & FLAG_FULLSCREEN)
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

        if (IsKeyPressed(bind_pause) && (flag & FLAG_WORLD_LOADED))
        {
            if (flag & FLAG_WORLD_LOADED)
            {
                if (!state_menu_depth)
                {
                    state_menu_depth = 1;
                    menu_index = MENU_GAME;
                    flag |= FLAG_PAUSED;
                    player->container_state = 0;
                    show_cursor;
                }
                else if (state_menu_depth == 1)
                    btn_func_back_to_game();
            }
            else
            {
                if (state_menu_depth == 1)
                    flag &= ~FLAG_ACTIVE;
                else btn_func_back();
            }
        }

        setting.render_size = (v2f32){GetRenderWidth(), GetRenderHeight()};
        update_render_settings(setting.render_size);

        BeginDrawing();
        update_menus(setting.render_size);
        EndDrawing();

        if (!(flag & FLAG_WORLD_LOADED))
            goto section_menu_title;

        if (!(flag & FLAG_PAUSED) && (flag & FLAG_WORLD_LOADED))
            break;
    }
    return 0;
}

void
draw_gui()
{
    if (state_menu_depth && lily.container_state)
        draw_containers(&lily, setting.render_size);
}
