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
u16 state = 0;
u8 stateMenuDepth = 0;
static f64 gameStartTime = 0;
static u64 gameTick = 0;
static u64 gameDays = 0;
settings setting =
{
    .reachDistance =        SETTING_REACH_DISTANCE_MAX,
    .fov =                  SETTING_FOV_DEFAULT,
    .mouseSensitivity =     SETTING_MOUSE_SENSITIVITY_DEFAULT/650.0f,
    .renderDistance =       SETTING_RENDER_DISTANCE_DEFAULT,
    .guiScale =             SETTING_GUI_SCALE_DEFAULT,
};
Player lily =
{
    .name = "Lily",
    .pos = {0.0f},
    .scl = {0.6f, 0.6f, 1.8f},
    .collisionCheckStart = {0.0f},
    .collisionCheckEnd = {0.0f},
    .pitch = -29.0f,
    .yaw = 121.0f,
    .sinPitch = 0.0f, .cosPitch = 0.0f, .sinYaw = 0.0f, .cosYaw = 0.0f,
    .eyeHeight = 1.5f,
    .m = 2.0f,
    .movementSpeed = 10.0f,
    .containerState = 0,
    .perspective = 0,

    .camera =
    {
        .up.z = 1.0f,
        .fovy = 70.0f,
        .projection = CAMERA_PERSPECTIVE,
    },
    .cameraDistance = SETTING_CAMERA_DISTANCE_MAX,
    .cameraDebugInfo =
    {
        .up.z = 1.0f,
        .fovy = 50.0f,
        .projection = CAMERA_ORTHOGRAPHIC,
    },

    .spawnPoint = {0.0f, 0.0f, 0.0f},
};

// ---- signatures -------------------------------------------------------------
void update_world();
void update_input_general(Player *player);
void update_input_world(Player *player);
void draw_skybox();

int main(int argc, char **argv)
    // ---- game init ----------------------------------------------------------
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
    update_render_settings(renderSize);
    init_super_debugger(renderSize);

    setting.renderDistance = SETTING_RENDER_DISTANCE_MAX; //temp

    state |= STATE_ACTIVE;
    while (!WindowShouldClose() && (state & STATE_ACTIVE))
        // ---- game loop ------------------------------------------------------
    {
        update_input_general(&lily);
        update_render_settings(renderSize);
        renderSize = (v2f32){GetRenderWidth(), GetRenderHeight()};
        BeginDrawing();
        //draw_texture_tiled(); // TODO: draw tiled texture of title screen
        ClearBackground(DARKBROWN); // TODO: make actual panoramic scene
        update_menus(renderSize);
        if (state & STATE_WORLD_LOADED)
        {
            mouseDelta = GetMouseDelta();
            draw_skybox();
            update_world();
        }
        EndDrawing();

        if (state & STATE_PAUSED) // TODO: make real pausing instead of using the uncleared bg as still
        {
            BeginDrawing();
            draw_menu_overlay(renderSize);
            EndDrawing();

            while ((state & STATE_PAUSED) && (state & STATE_ACTIVE) && !WindowShouldClose())
            {
                renderSize = (v2f32){GetRenderWidth(), GetRenderHeight()};
                update_input_general(&lily);
                BeginDrawing();
                update_menus(renderSize);
                EndDrawing();
            }
        }
    }

    // ---- game close ---------------------------------------------------------
    UnloadTexture(cobblestone); //temp
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

    { //temp
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
        cobblestone = LoadTexture("resources/textures/blocks/stone.png"); //temp
        dirt = LoadTexture("resources/textures/blocks/dirt.png"); //temp
    }
    lily.state |= STATE_FALLING; //temp
    lily.state &= ~STATE_PARSE_TARGET;

    state |= STATE_HUD | STATE_WORLD_LOADED;
    LOGINFO("%s", "World Loaded: Poop Consistency Tester");
}

void update_world()
{
    gameTick = (floor((get_time_ms() - gameStartTime)*20)) - (SETTING_DAY_TICKS_MAX*gameDays);
    if (gameTick >= SETTING_DAY_TICKS_MAX)
        ++gameDays;

    if (MODE_COLLIDE)
        update_collision_static(&lily);

    if (state & STATE_DEBUG)
        update_camera_movements_debug_info(&lily.cameraDebugInfo, &lily);

    if (stateMenuDepth || state & STATE_SUPER_DEBUG)
        show_cursor;
    else hide_cursor;

    update_input_world(&lily);
    update_player_states(&lily);
    update_camera_movements_player(&lily);
    BeginMode3D(lily.camera);
    draw_chunk_buffer(*chunkBuf);

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

        if (targetChunk != NULL && lily.state & STATE_PARSE_TARGET && (state & STATE_HUD))
        {
            if (targetChunk->i
                    [lily.lastTarget.z - WORLD_BOTTOM]
                    [lily.lastTarget.y - (targetChunk->pos.y*CHUNK_SIZE)]
                    [lily.lastTarget.x - (targetChunk->pos.x*CHUNK_SIZE)] & NOT_EMPTY)
            {
                draw_block_wires(&lily.lastTarget);
                if (state & STATE_DEBUG)
                    DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}), lily.camera.target, RED);
            }
            else if (state & STATE_DEBUG)
                DrawLine3D(Vector3Subtract(lily.camera.position, (Vector3){0.0f, 0.0f, 0.5f}), lily.camera.target, GREEN);
        }
    }

    if (state & STATE_DEBUG_MORE)
    {
        /*temp
          draw_block_wires(&target_coordinates_feet);
          printf("feet: %d %d %d\n", target_coordinates_feet.x, target_coordinates_feet.y, target_coordinates_feet.z);
          */
        DrawCubeWiresV(lily.camera.target, (Vector3){1, 1, 1}, GREEN);
        draw_bounding_box(&lily.pos, &lily.scl);
        draw_bounding_box_clamped(&lily.pos, &lily.scl); //temp AABB collision
        draw_default_grid(COL_X, COL_Y, COL_Z);
    }

    EndMode3D();

    if (state & STATE_HUD)
    {
        draw_hud();
        draw_debug_info(&lily.cameraDebugInfo);
        if (stateMenuDepth && lily.containerState)
            draw_containers(&lily, renderSize);
    }

    draw_super_debugger(renderSize);
}

void update_input_general(Player *player)
{
    if (IsKeyPressed(bindToggleFullscreen))
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

    if (IsKeyPressed(bindPause) && (state & STATE_WORLD_LOADED))
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
            btn_func_back_to_game();
    }

    else if (IsKeyPressed(bindPause) && !(state & STATE_WORLD_LOADED))
    {
        if (stateMenuDepth == 1)
            state &= ~STATE_ACTIVE;
        else btn_func_back();
    }

    // ---- debug --------------------------------------------------------------
#if RELEASE_BUILD == 0
    if (IsKeyPressed(KEY_TAB))
        state ^= STATE_SUPER_DEBUG;
#endif // RELEASE_BUILD
}

void update_input_world(Player *player)
{
    // ---- jumping ------------------------------------------------------------
    if (IsKeyDown(bindJump))
    {
        if (IsKeyPressed(bindJump) && get_double_press(player, bindJump))
            player->state ^= STATE_FLYING;

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
        player->v.x -= player->movementSpeed*player->sinYaw;
        player->v.y += player->movementSpeed*player->cosYaw;
    }

    if (IsKeyDown(bindStrafeRight))
    {
        player->v.x += player->movementSpeed*player->sinYaw;
        player->v.y -= player->movementSpeed*player->cosYaw;
    }

    if (IsKeyDown(bindWalkBackwards))
    {
        player->v.x -= player->movementSpeed*player->cosYaw;
        player->v.y -= player->movementSpeed*player->sinYaw;
    }

    if (IsKeyDown(bindWalkForwards))
    {
        if (IsKeyPressed(bindWalkForwards) && get_double_press(player, bindWalkForwards))
            player->state |= STATE_SPRINTING;

        player->v.x += player->movementSpeed*player->cosYaw;
        player->v.y += player->movementSpeed*player->sinYaw;
    }

    player->movementStepLength = sqrt(sqr(player->v.x) + sqr(player->v.y));
    if (player->movementStepLength > 0.0f)
    {
        player->v.x /= player->movementStepLength;
        player->v.y /= player->movementStepLength;

        player->pos.x += player->v.x*player->movementSpeed;
        player->pos.y += player->v.y*player->movementSpeed;
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
    {
        if (state & STATE_DEBUG)
            state &= ~(STATE_DEBUG | STATE_DEBUG_MORE);
        else state |= STATE_DEBUG;

        if (IsKeyDown(KEY_LEFT_SHIFT) && (state & STATE_DEBUG))
            state |= STATE_DEBUG_MORE;
    }

    if (IsKeyPressed(bindTogglePerspective))
    {
        if (player->perspective < 4)
            ++player->perspective;
        else player->perspective = 0;
    }

    if (!stateMenuDepth && !(state & STATE_SUPER_DEBUG))
    {
        hide_cursor;
        center_cursor;
    }
}

f64 skyboxTime = 0;
f64 skyboxMidDay = 0;
f64 skyboxPreBurn = 0;
f64 skyboxBurn = 0;
f64 skyboxBurnBoost = 0;
f64 skyboxMidNight = 0;
Color skyboxRGBA = {0};
void draw_skybox()
{
    skyboxTime =        (f64)gameTick/SETTING_DAY_TICKS_MAX;
    skyboxMidDay =      fabs(sinf(1.5f*powf(sinf(skyboxTime*PI), 2.0f)));
    skyboxPreBurn =     fabs(sinf(powf(sinf((skyboxTime + 0.33)*PI*1.2f), 16.0f)));
    skyboxBurn =        fabs(sinf(1.5f*powf(sinf((skyboxTime + 0.124f)*PI*1.6f), 20.0f)));
    skyboxBurnBoost =   fabs(powf(sinf((skyboxTime + 0.212f)*PI*1.4f), 64.0f));
    skyboxMidNight =    fabs(sinf(powf(2*cosf(skyboxTime*PI), 3.0f)));
    skyboxRGBA =
        (Color){
            Clamp((skyboxMidDay*171) + (skyboxBurn*85) + (skyboxMidNight*1) +   (skyboxPreBurn*13) +    (skyboxBurnBoost*76),   0, 255),
            Clamp((skyboxMidDay*229) + (skyboxBurn*42) + (skyboxMidNight*4) +   (skyboxPreBurn*7) +     (skyboxBurnBoost*34),   0, 255),
            Clamp((skyboxMidDay*255) + (skyboxBurn*19) + (skyboxMidNight*14) +  (skyboxPreBurn*20),                             0, 255),
            255
        };

    ClearBackground(skyboxRGBA);
}
