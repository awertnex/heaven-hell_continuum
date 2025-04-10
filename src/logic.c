#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "h/logic.h"
#include "h/chunking.h"

// ---- variables --------------------------------------------------------------
v3i32 targetCoordinatesFeet; /*temp*/

bool get_double_press(Player *player, KeyboardKey key)
{
    static f64 doublePressStartTime = 0;
    static KeyboardKey doublePressKey = KEY_NULL;

    if (((player->state & STATE_DOUBLE_PRESS) && get_time_ms() - doublePressStartTime >= 0.25f) ||
            key != doublePressKey)
        player->state &= ~STATE_DOUBLE_PRESS;
    else
    {
        doublePressKey = KEY_NULL;
        player->state &= ~STATE_DOUBLE_PRESS;
        return true;
    }

    if (!(player->state & STATE_DOUBLE_PRESS))
    {
        player->state |= STATE_DOUBLE_PRESS;
        doublePressKey = key;
        doublePressStartTime = get_time_ms();
    }
    return false;
}

void update_player_states(Player *player)
{
    if (!(player->state & STATE_CAN_JUMP) && !(player->state & STATE_FLYING))
        give_gravity(player);

    if (player->state & STATE_FLYING)
    {
        player->v = v3fzero;
        player->movementSpeed = PLAYER_SPEED_FLY*dt;
        player->camera.fovy = 80; //TODO: revise (add lerp)
    }

    if (player->state & STATE_SNEAKING && !(player->state & STATE_FLYING))
        player->movementSpeed = PLAYER_SPEED_SNEAK*dt;
    else if (player->state & STATE_SPRINTING)
    {
        !(player->state & STATE_FLYING) ?
            (player->movementSpeed = PLAYER_SPEED_SPRINT*dt,
             player->camera.fovy = 75) :

            (player->movementSpeed = PLAYER_SPEED_FLY_FAST*dt,
             player->camera.fovy = 90);
    }
    else if (!(player->state & STATE_SNEAKING) && !(player->state & STATE_SPRINTING) && !(player->state & STATE_FLYING))
    {
        player->movementSpeed = PLAYER_SPEED_WALK*dt;
        player->camera.fovy = 70;
    }

    //TODO: revise (actually kill the player)
    if (player->pos.z < WORLD_KILL_Z)
    {
        player->pos =
            (Vector3){
                player->spawnPoint.x,
                player->spawnPoint.y,
                player->spawnPoint.z,
            };
        player->v.z = 0;
    }
}

void update_camera_movements_player(Player *player)
{
    if (!(stateMenuDepth) && !(state & STATE_SUPER_DEBUG))
    {
        player->yaw -= GetMouseDelta().x*((f32)setting.mouseSensitivity/650);
        if (player->pitch <= 90 && player->pitch >= -90)
            player->pitch -= GetMouseDelta().y*((f32)setting.mouseSensitivity/650);
    }

    if (player->yaw >= 360)         player->yaw = 0;
    else if (player->yaw < 0)       player->yaw += 360;
    if (player->pitch > 90)         player->pitch = 90;
    else if (player->pitch < -90)   player->pitch = -90;

    switch (player->perspective)
    {
        case 0: // ---- 1st person ---------------------------------------------
            player->camera.position =
                (Vector3){player->pos.x, player->pos.y, player->pos.z + 1.5f,
                };
            player->camera.target =
                (Vector3){
                    player->pos.x + ((cosf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD))*setting.reachDistance),
                    player->pos.y + ((sinf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD))*setting.reachDistance),
                    player->camera.position.z + (sinf(player->pitch*MC_C_DEG2RAD)*setting.reachDistance),
                };
            player->camera.up =
                (Vector3){
                    -cosf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                    -sinf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                    cosf(player->pitch*MC_C_DEG2RAD),
                };
            break;

        case 1: // ---- 3rd person back ----------------------------------------
            player->camera.position =
                (Vector3){
                    player->pos.x - ((cosf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD))*player->cameraDistance),
                    player->pos.y - ((sinf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD))*player->cameraDistance),
                    player->pos.z + 1.5f - (sinf(player->pitch*MC_C_DEG2RAD)*player->cameraDistance),
                };
            player->camera.target =
                (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + 1.5f,
                };
            player->camera.up =
                (Vector3){
                    -cosf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                    -sinf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                    cosf(player->pitch*MC_C_DEG2RAD),
                };
            break;

        case 2: // ---- 3rd person front ---------------------------------------
            player->camera.position =
                (Vector3){
                    player->pos.x + ((cosf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD))*player->cameraDistance),
                    player->pos.y + ((sinf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD))*player->cameraDistance),
                    player->pos.z + 1.5f + (sinf(player->pitch*MC_C_DEG2RAD)*player->cameraDistance),
                };
            player->camera.target =
                (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + 1.5f,
                };
            player->camera.up =
                (Vector3){
                    -cosf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                    -sinf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                    cosf(player->pitch*MC_C_DEG2RAD),
                };
            break;

        case 3: // ---- 3rd person stalker -------------------------------------
            player->camera.target =
                (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + 1.5f,
                };
            break;

        case 4: // ---- spectator ----------------------------------------------
            break;
    }
}

void update_camera_movements_debug_info(Camera3D *camera, Player *player)
{
    if (player->perspective == 0 || player->perspective == 1)
    {
        camera->position =
            (Vector3){
                -cosf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD),
                -sinf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD),
                -sinf(player->pitch*MC_C_DEG2RAD),
            };
        camera->up =
            (Vector3){
                -cosf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                -sinf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                cosf(player->pitch*MC_C_DEG2RAD),
            };
        camera->target = Vector3Zero();
    }
    else if (player->perspective == 2)
    {
        camera->position =
            (Vector3){
                cosf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD),
                sinf(player->yaw*MC_C_DEG2RAD)*cosf(player->pitch*MC_C_DEG2RAD),
                sinf(player->pitch*MC_C_DEG2RAD),
            };
        camera->target = Vector3Zero();
        camera->up =
            (Vector3){
                -cosf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                -sinf(player->yaw*MC_C_DEG2RAD)*sinf(player->pitch*MC_C_DEG2RAD),
                cosf(player->pitch*MC_C_DEG2RAD),
            };
    }
}

bool check_target_delta_position(Vector3 *coordinates, v3i32 *lastTarget)
{
    if (lastTarget->x == floorf(coordinates->x) &&
            lastTarget->y == floorf(coordinates->y) &&
            lastTarget->z == floorf(coordinates->z))
        return false;

    lastTarget->x = (i32)floorf(coordinates->x);
    lastTarget->y = (i32)floorf(coordinates->y);
    lastTarget->z = (i32)floorf(coordinates->z);
    return true;
}

bool is_range_within_ff(f32 *pos, f32 start, f32 end)
{
    if (*pos < start || *pos > end) return false;
    return true;
}

bool is_range_within_v2ff(v2f32 *pos, v2f32 start, v2f32 end)
{
    if (pos->x < start.x || pos->x > end.x ||
            pos->y < start.y || pos->y > end.y) return false;
    return true;
}

bool is_range_within_v3fi(Vector3 *pos, v3i32 start, v3i32 end)
{
    if (pos->x < start.x || pos->x > end.x ||
            pos->y < start.y || pos->y > end.y ||
            pos->z < start.z || pos->z > end.z) return false;
    return true;
}

b8 is_ray_intersect(Player *player) //TODO: make the player ray intersection
{
    if (targetChunk->i[player->lastTarget.z][player->lastTarget.y][player->lastTarget.x])
        return true;
    return false;
}

void give_gravity(Player *player)
{
    if (player->state & STATE_FALLING)
        player->v.z -= (PLAYER_JUMP_HEIGHT*GRAVITY*player->m*dt);
    player->pos.z += player->v.z;
    //printf("   previous time: %.2lf   delta time: %.2lf\n", get_time_ms(), get_delta_time(&start_time)); //temp
}

void give_collision_static(Player *player, v3i32 *targetCoordinatesFeet)
{
    i32 x = floorf(player->pos.x);
    i32 y = floorf(player->pos.y);
    i32 z = floorf(player->pos.z);
    Chunk *targetChunkFeet = get_chunk(&player->lastPos, &player->state, STATE_PARSE_COLLISION_FEET);
    if ((player->state & STATE_PARSE_COLLISION_FEET) && player->pos.z > WORLD_BOTTOM)
    {
        if (targetChunkFeet->i
                [z - 1 - WORLD_BOTTOM]
                [y - (targetChunkFeet->pos.y*CHUNK_SIZE)]
                [x - (targetChunkFeet->pos.x*CHUNK_SIZE)] & NOT_EMPTY)
        {
            player->pos.z = ceilf(targetCoordinatesFeet->z) + WORLD_BOTTOM + 1;
            player->v.z = 0;
            if (player->state & STATE_FLYING) player->state &= ~STATE_FLYING;
            player->state |= STATE_CAN_JUMP;
            player->state &= ~STATE_FALLING;
        }
        else player->state |= STATE_FALLING;
    }
    //TODO: move to new 'void parse_camera_collisions()'
    player->cameraDistance = SETTING_CAMERA_DISTANCE_MAX;
}

f64 get_time_ms()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec + (f64)tp.tv_usec/1000000;
}

bool get_timer(f64 *timeStart, f32 interval)
{
    if (get_time_ms() - *timeStart >= interval)
    {
        *timeStart = get_time_ms();
        return true;
    }
    return false;
}

void draw_default_grid(Color x, Color y, Color z)
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

    draw_line_3d(v3izero, (v3i32){2, 0, 0}, x);
    draw_line_3d(v3izero, (v3i32){0, 2, 0}, y);
    draw_line_3d(v3izero, (v3i32){0, 0, 2}, z);
    rlEnd();
    rlPopMatrix();
}
