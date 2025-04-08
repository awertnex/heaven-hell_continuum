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
v2f32 renderSize = {WIDTH, HEIGHT};
f64 startTime = 0;
static f64 gameStartTime = 0;
static u64 gameTick = 0;
static u64 gameDays = 0;
u16 state = 0;
u8 stateMenuDepth = 0;
settings setting =
{
    .reachDistance =        SETTING_REACH_DISTANCE_MAX,
    .fov =                  SETTING_FOV_DEFAULT,
    .mouseSensitivity =     SETTING_MOUSE_SENSITIVITY_DEFAULT,
    .renderDistance =       SETTING_RENDER_DISTANCE_DEFAULT,
    .guiScale =             SETTING_GUI_SCALE_DEFAULT,
};
Player lily =
{
    .name = "Lily",
    .pos = {8, -6, -65},
    .scl = {0.6f, 0.6f, 1.8f},
    .pitch = -29,
    .yaw = 121,
    .m = 2,
    .movementSpeed = 10,
    .containerState = 0,
    .perspective = 0,

    .camera =
    {
        .up.z = 1,
        .fovy = 70,
        .projection = CAMERA_PERSPECTIVE,
    },
    .cameraDistance = SETTING_CAMERA_DISTANCE_MAX,

    .spawnPoint = {3, -6, 9},
};

// ---- signatures -------------------------------------------------------------
static void update_world();
static void update_input(Player *player);

int main(int argc, char **argv) // ---- game init ------------------------------
{
    if (MODE_DEBUG)
        LOGDEBUG("%s", "Debugging Enabled");

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

    InitWindow(WIDTH, HEIGHT, "minecraft.c");
    init_fonts();
    init_gui();
    apply_render_settings(renderSize);
    init_super_debugger();

    // TODO: fix rendering issues when resizing window
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(200, 150);

    setting.renderDistance = SETTING_RENDER_DISTANCE_MAX; //temp

    state |= STATE_ACTIVE;
    while (state & STATE_ACTIVE) // ---- game loop -----------------------------
    {
        if (!(state & STATE_WORLD_LOADED))
        {
            BeginDrawing();
            ClearBackground(DARKBROWN); /* TODO: make actual panoramic scene */
            EndDrawing();
            update_menus(renderSize);
        }
        else
        {
            update_world();
        }

        if (state & STATE_PAUSED)
        {
            BeginDrawing();
            draw_menu_overlay(renderSize);
            EndDrawing();

            while (state & STATE_PAUSED && state & STATE_ACTIVE)
            {
                BeginDrawing();
                update_menus(renderSize);
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
    gameStartTime = get_time_ms();
    hide_cursor;
    center_cursor;

    init_chunking();

    { /*temp*/
        chunkBuf[0][0].pos = (v2i16){0, 0};
        chunkBuf[0][1].pos = (v2i16){0, -1};
        chunkBuf[0][2].pos = (v2i16){0, -2};
        chunkBuf[0][3].pos = (v2i16){-1, 0};
        chunkBuf[0][4].pos = (v2i16){0, 1};
        chunkBuf[0][5].pos = (v2i16){0, 2};
        chunkBuf[0][6].pos = (v2i16){-1, -1};
        chunkBuf[0][7].pos = (v2i16){1, 0};
        parse_chunk_states(&chunkBuf[0][0], 3);
        parse_chunk_states(&chunkBuf[0][1], 2);
        parse_chunk_states(&chunkBuf[0][2], 2);
        parse_chunk_states(&chunkBuf[0][3], 30);
        parse_chunk_states(&chunkBuf[0][4], 2);
        parse_chunk_states(&chunkBuf[0][5], 2);
        parse_chunk_states(&chunkBuf[0][6], 3);
        parse_chunk_states(&chunkBuf[0][7], 2);
    }
    lily.state |= STATE_FALLING; //temp
    lily.state &= ~STATE_PARSE_TARGET;

    if (MODE_DEBUG)
        lily.state |= STATE_FLYING;

    state |= STATE_HUD | STATE_WORLD_LOADED;
    LOGINFO("%s", "World Loaded: Poop Consistency Tester");
}

void update_world()
{
    startTime = get_time_ms();
    gameTick = (u64)((get_time_ms() - gameStartTime)*20);
    if (gameTick >= SETTING_DAY_TICKS_MAX)
    {
	gameTick = 0;
	++gameDays;
    }
    printf("tick: %lu\n", gameTick);
    renderSize = (v2f32){GetRenderWidth(), GetRenderHeight()};

    parse_player_states(&lily);

    if (MODE_COLLIDE)
        give_collision_static(&lily, &targetCoordinatesFeet);

    if (state & STATE_DEBUG)
        give_camera_movements_debug_info(&cameraDebugInfo, &lily);
    give_camera_movements_player(&lily);

    if (stateMenuDepth || state & STATE_SUPER_DEBUG)
        show_cursor;
    else hide_cursor;

    update_input(&lily);

    BeginDrawing();
    ClearBackground(COL_SKYBOX); /* TODO: make actual skybox */
    BeginMode3D(lily.camera);
    { /*temp*/
        draw_chunk(&chunkBuf[0][0], 20);
        draw_chunk(&chunkBuf[0][1], 2);
        draw_chunk(&chunkBuf[0][2], 2);
        draw_chunk(&chunkBuf[0][3], 30);
        draw_chunk(&chunkBuf[0][4], 2);
        draw_chunk(&chunkBuf[0][5], 2);
        draw_chunk(&chunkBuf[0][6], 9);
        draw_chunk(&chunkBuf[0][7], 2);
    }

    //TODO: make a function 'index_to_bounding_box()'
    //if (GetRayCollisionBox(GetScreenToWorldRay(cursor, lily.camera), (BoundingBox){&lily.previous_target}).hit)
    {
    }
    if (is_range_within_v3fi(&lily.camera.target,
                (v3i32){-WORLD_SIZE, -WORLD_SIZE, WORLD_BOTTOM},
                (v3i32){WORLD_SIZE, WORLD_SIZE, worldHeight}))
    {
        if (check_target_delta_position(&lily.camera.target, &lily.lastTarget))
            targetChunk = get_chunk(&lily.lastTarget, &lily.state, STATE_PARSE_TARGET);

        if (targetChunk != NULL && lily.state & STATE_PARSE_TARGET)
        {
            if (targetChunk->i
                    [lily.lastTarget.z - WORLD_BOTTOM]
                    [lily.lastTarget.y - (targetChunk->pos.y*CHUNK_SIZE)]
                    [lily.lastTarget.x - (targetChunk->pos.x*CHUNK_SIZE)] & NOT_EMPTY)
            {
                draw_block_wires(&lily.lastTarget);
                DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}), lily.camera.target, RED);
            }
            else DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}), lily.camera.target, GREEN);
        }
    }

    if (MODE_DEBUG)
    {
        /*temp
          draw_block_wires(&target_coordinates_feet);
          printf("feet: %d %d %d\n", target_coordinates_feet.x, target_coordinates_feet.y, target_coordinates_feet.z);
          */
        DrawCubeWiresV(lily.camera.target, (Vector3){1, 1, 1}, GREEN);
        draw_bounding_box(&lily.pos, &lily.scl);
        draw_default_grid(COL_X, COL_Y, COL_Z);
    }

    EndMode3D();

    if (state & STATE_HUD)
    {
        draw_hud();
        draw_debug_info();
        if (stateMenuDepth)
        {
            if (lily.containerState & CONTR_INVENTORY)
                draw_inventory(renderSize);
        }
    }

    draw_super_debugger();
    EndDrawing();
}

void update_input(Player *player)
{
    // ---- jumping ------------------------------------------------------------
    if (IsKeyPressed(bindJump))
        if (get_double_press(player, bindJump))
            player->state ^= STATE_FLYING;

    if (IsKeyDown(bindJump))
    {
        if (player->state & STATE_FLYING)
            player->pos.z += player->movementSpeed;

        if (player->state & STATE_CAN_JUMP)
        {
            player->v.z += PLAYER_JUMP_HEIGHT;
            player->state &= ~STATE_CAN_JUMP;
        }
    }

    // ---- sneaking -----------------------------------------------------------
    if (IsKeyDown(bindSneak))
    {
        if (player->state & STATE_FLYING)
            player->pos.z -= player->movementSpeed;
        else
            player->state |= STATE_SNEAKING;
    }

    if (IsKeyUp(bindSneak))
        player->state &= ~STATE_SNEAKING;

    // ---- sprinting ----------------------------------------------------------
    if (IsKeyDown(bindSprint) && IsKeyDown(bindWalkForwards))
        player->state |= STATE_SPRINTING;

    if (IsKeyUp(bindSprint) && IsKeyUp(bindWalkForwards))
        player->state &= ~STATE_SPRINTING;

    // ---- moving -------------------------------------------------------------
    if (IsKeyDown(bindStrafeLeft))
    {
        player->pos.x -= player->movementSpeed*sinf(player->yaw*DEG2RAD);
        player->pos.y += player->movementSpeed*cosf(player->yaw*DEG2RAD);
    }

    if (IsKeyDown(bindStrafeRight))
    {
        player->pos.x += player->movementSpeed*sinf(player->yaw*DEG2RAD);
        player->pos.y -= player->movementSpeed*cosf(player->yaw*DEG2RAD);
    }

    if (IsKeyDown(bindWalkBackwards))
    {
        player->pos.x -= player->movementSpeed*cosf(player->yaw*DEG2RAD);
        player->pos.y -= player->movementSpeed*sinf(player->yaw*DEG2RAD);
    }

    if (IsKeyPressed(bindWalkForwards))
        if (get_double_press(player, bindWalkForwards))
            player->state |= STATE_SPRINTING;

    if (IsKeyDown(bindWalkForwards))
    {
        player->pos.x += player->movementSpeed*cosf(player->yaw*DEG2RAD);
        player->pos.y += player->movementSpeed*sinf(player->yaw*DEG2RAD);
    }

    // ---- gameplay -----------------------------------------------------------
    if (IsMouseButtonDown(bindAttackOrDestroy))
    {
        if (player->state & STATE_PARSE_TARGET)
            remove_block(targetChunk, lily.lastTarget.x, lily.lastTarget.y, floorf(lily.lastTarget.z - WORLD_BOTTOM));
    }

    if (IsMouseButtonDown(bindPickBlock))
    {
    }

    if (IsMouseButtonDown(BindUseItemOrPlaceBlock))
    {
        if (player->state & STATE_PARSE_TARGET)
            add_block(targetChunk, lily.lastTarget.x, lily.lastTarget.y, floorf(lily.lastTarget.z - WORLD_BOTTOM));
    }

    // ---- inventory ----------------------------------------------------------
    if (IsKeyPressed(bindHotbarSlot1) || IsKeyPressed(bindHotbarSlotKP1))
        hotbarSlotSelected = 1;

    if (IsKeyPressed(bindHotbarSlot2) || IsKeyPressed(bindHotbarSlotKP2))
        hotbarSlotSelected = 2;

    if (IsKeyPressed(bindHotbarSlot3) || IsKeyPressed(bindHotbarSlotKP3))
        hotbarSlotSelected = 3;

    if (IsKeyPressed(bindHotbarSlot4) || IsKeyPressed(bindHotbarSlotKP4))
        hotbarSlotSelected = 4;

    if (IsKeyPressed(bindHotbarSlot5) || IsKeyPressed(bindHotbarSlotKP5))
        hotbarSlotSelected = 5;

    if (IsKeyPressed(bindHotbarSlot6) || IsKeyPressed(bindHotbarSlotKP6))
        hotbarSlotSelected = 6;

    if (IsKeyPressed(bindHotbarSlot7) || IsKeyPressed(bindHotbarSlotKP7))
        hotbarSlotSelected = 7;

    if (IsKeyPressed(bindHotbarSlot8) || IsKeyPressed(bindHotbarSlotKP8))
        hotbarSlotSelected = 8;

    if (IsKeyPressed(bindHotbarSlot9) || IsKeyPressed(bindHotbarSlotKP9))
        hotbarSlotSelected = 9;

    if (IsKeyPressed(bindHotbarSlot0) || IsKeyPressed(bindHotbarSlotKP0))
        hotbarSlotSelected = 10;

    if (IsKeyPressed(bindOpenOrCloseInventory))
    {
        if (player->containerState & CONTR_INVENTORY && stateMenuDepth)
        {
            stateMenuDepth = 0;
            player->containerState &= ~CONTR_INVENTORY;
        }
        else if (!(player->containerState & CONTR_INVENTORY) && !stateMenuDepth)
        {
            stateMenuDepth = 1;
            player->containerState |= CONTR_INVENTORY;
        }

        if (!(player->containerState & CONTR_INVENTORY) && stateMenuDepth)
            --stateMenuDepth;
    }

    // ---- miscellaneous ------------------------------------------------------
    if (IsKeyPressed(bindToggleHUD))
        state ^= STATE_HUD;

    if (IsKeyPressed(bindToggleDebug))
        state ^= STATE_DEBUG;

    if (IsKeyPressed(bindToggleFullscreen))
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
        apply_render_settings(renderSize);
    }

    if (IsKeyPressed(bindTogglePerspective))
    {
        if (player->perspective < 4)
            ++player->perspective;
        else player->perspective = 0;
    }

    if (IsKeyPressed(bindPause))
    {
        if (!stateMenuDepth)
        {
            stateMenuDepth = 1;
            menuIndex = MENU_GAME;
            state |= STATE_PAUSED;
            player->containerState = 0;
            show_cursor;
        }
        else if (stateMenuDepth == 1)
        {
            stateMenuDepth = 0;
            player->containerState = 0;
        }
        else --stateMenuDepth;
    }
    if (!stateMenuDepth && !(state & STATE_SUPER_DEBUG))
    {
        hide_cursor;
        center_cursor;
    }

    // ---- debug --------------------------------------------------------------
    if (IsKeyPressed(KEY_TAB))
        state ^= STATE_SUPER_DEBUG;

    if (IsKeyPressed(bindQuit))
        state &= ~STATE_ACTIVE;
}
