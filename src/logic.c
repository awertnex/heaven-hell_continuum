#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "h/logic.h"
#include "h/main.h"
#include "h/chunking.h"
#include "h/gui.h"

// ---- variables --------------------------------------------------------------
player lily =
{
    .name = "Lily",
    .pos = {8, -6, -65},
    .scl = {0.6f, 0.6f, 1.8f},
    .pitch = -29,
    .yaw = 121,
    .m = 2,
    .movement_speed = 10,
    .container_state = 0,
    .perspective = 0,

    .camera =
    {
        .up.z = 1,
        .fovy = 70,
        .projection = CAMERA_PERSPECTIVE,
    },
    .camera_distance = SETTING_CAMERA_DISTANCE_MAX,

    .spawn_point = {3, -6, 9},
};

v3i32 target_coordinates_feet; /*temp*/

// ---- functions --------------------------------------------------------------
bool get_double_press(player *player, KeyboardKey key)
{
    static f64 double_press_start_time = 0;
    static KeyboardKey double_press_key = KEY_NULL;

    if (((player->state & STATE_DOUBLE_PRESS) && get_time_ms() - double_press_start_time >= 0.25f) ||
            key != double_press_key)
        player->state &= ~STATE_DOUBLE_PRESS;
    else
    {
        double_press_key = KEY_NULL;
        player->state &= ~STATE_DOUBLE_PRESS;
        return true;
    }

    if (!(player->state & STATE_DOUBLE_PRESS))
    {
        player->state |= STATE_DOUBLE_PRESS;
        double_press_key = key;
        double_press_start_time = get_time_ms();
    }
    return false;
}

void parse_player_states(player *player)
{
    if (!(player->state & STATE_CAN_JUMP) && !(player->state & STATE_FLYING))
        give_gravity(player);

    if (player->state & STATE_FLYING)
    {
        player->v = v3fzero;
        player->movement_speed = PLAYER_SPEED_FLY*dt;
        player->camera.fovy = 80; //TODO: revise (add lerp)
    }

    if (player->state & STATE_SNEAKING && !(player->state & STATE_FLYING))
        player->movement_speed = PLAYER_SPEED_SNEAK*dt;
    else if (player->state & STATE_SPRINTING)
    {
        !(player->state & STATE_FLYING) ?
            (player->movement_speed = PLAYER_SPEED_SPRINT*dt,
             player->camera.fovy = 75) :

            (player->movement_speed = PLAYER_SPEED_FLY_FAST*dt,
             player->camera.fovy = 90);
    }
    else if (!(player->state & STATE_SNEAKING) && !(player->state & STATE_SPRINTING) && !(player->state & STATE_FLYING))
    {
        player->movement_speed = PLAYER_SPEED_WALK*dt;
        player->camera.fovy = 70;
    }

    //TODO: revise (actually kill the player)
    if (player->pos.z < WORLD_KILL_Z)
    {
        player->pos =
            (Vector3){
                player->spawn_point.x,
                player->spawn_point.y,
                player->spawn_point.z,
            };
        player->v.z = 0;
    }
}

void give_camera_movements_player(player *player)
{
    if (!(state_menu_depth) && !(state & STATE_SUPER_DEBUG))
    {
        player->yaw -= GetMouseDelta().x*((f32)setting.mouse_sensitivity/650);
        if (player->pitch <= 90 && player->pitch >= -90)
            player->pitch -= GetMouseDelta().y*((f32)setting.mouse_sensitivity/650);
    }

    if (player->yaw >= 360)         player->yaw = 0;
    else if (player->yaw < 0)       player->yaw += 360;
    if (player->pitch > 90)         player->pitch = 90;
    else if (player->pitch < -90)   player->pitch = -90;

    switch (lily.perspective)
    {
        case 0: // ---- 1st person ---------------------------------------------
            player->camera.position =
                (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + 1.5f,
                };
            player->camera.target =
                (Vector3){
                    player->pos.x + ((cosf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD))*setting.reach_distance),
                    player->pos.y + ((sinf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD))*setting.reach_distance),
                    player->camera.position.z + (sinf(player->pitch*DEG2RAD)*setting.reach_distance),
                };
            player->camera.up =
                (Vector3){
                    -cosf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                    -sinf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                    cosf(player->pitch*DEG2RAD),
                };
            break;
        case 1: // ---- 3rd person back ----------------------------------------
            player->camera.position =
                (Vector3){
                    player->pos.x - ((cosf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD))*player->camera_distance),
                    player->pos.y - ((sinf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD))*player->camera_distance),
                    player->pos.z + 1.5f - (sinf(player->pitch*DEG2RAD)*player->camera_distance),
                };
            player->camera.target =
                (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + 1.5f,
                };
            player->camera.up =
                (Vector3){
                    -cosf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                    -sinf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                    cosf(player->pitch*DEG2RAD),
                };
            break;
        case 2: // ---- 3rd person front ---------------------------------------
            player->camera.position =
                (Vector3){
                    player->pos.x + ((cosf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD))*player->camera_distance),
                    player->pos.y + ((sinf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD))*player->camera_distance),
                    player->pos.z + 1.5f + (sinf(player->pitch*DEG2RAD)*player->camera_distance),
                };
            player->camera.target =
                (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + 1.5f,
                };
            player->camera.up =
                (Vector3){
                    -cosf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                    -sinf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                    cosf(player->pitch*DEG2RAD),
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

void give_camera_movements_debug_info(player *player)
{
    if (player->perspective == 0 || player->perspective == 1)
    {
        camera_debug_info.position =
            (Vector3){
                -cosf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD),
                -sinf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD),
                -sinf(player->pitch*DEG2RAD),
            };
        camera_debug_info.up =
            (Vector3){
                -cosf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                -sinf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                cosf(player->pitch*DEG2RAD),
            };
        camera_debug_info.target = Vector3Zero();
    }
    else if (player->perspective == 2)
    {
        camera_debug_info.position =
            (Vector3){
                cosf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD),
                sinf(player->yaw*DEG2RAD)*cosf(player->pitch*DEG2RAD),
                sinf(player->pitch*DEG2RAD),
            };
        camera_debug_info.target = Vector3Zero();
        camera_debug_info.up =
            (Vector3){
                -cosf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                -sinf(player->yaw*DEG2RAD)*sinf(player->pitch*DEG2RAD),
                cosf(player->pitch*DEG2RAD),
            };
    }
}

bool check_target_delta_position(Vector3 *coordinates, v3i32 *previous_target)
{
    if (previous_target->x == floorf(coordinates->x) &&
            previous_target->y == floorf(coordinates->y) &&
            previous_target->z == floorf(coordinates->z))
        return false;

    previous_target->x = (i32)floorf(coordinates->x);
    previous_target->y = (i32)floorf(coordinates->y);
    previous_target->z = (i32)floorf(coordinates->z);
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

void give_gravity(player *player)
{
    if (player->state & STATE_FALLING)
        player->v.z -= (PLAYER_JUMP_HEIGHT*GRAVITY*player->m*dt);
    player->pos.z += player->v.z;
    //printf("   previous time: %.2lf   delta time: %.2lf\n", get_time_ms(), get_delta_time(&start_time)); //temp
}

void give_collision_static(player *player, v3i32 *target_coordinates_feet)
{
    i32 x = floorf(player->pos.x);
    i32 y = floorf(player->pos.y);
    i32 z = floorf(player->pos.z);
    Chunk *target_chunk_feet = get_chunk(&player->previous_pos, &player->state, STATE_PARSE_COLLISION_FEET);
    if ((player->state & STATE_PARSE_COLLISION_FEET) && player->pos.z > WORLD_BOTTOM)
    {
        if (target_chunk_feet->i[z - 1 - WORLD_BOTTOM][y][x] & NOT_EMPTY)
        {
            player->pos.z = ceilf(target_coordinates_feet->z) + WORLD_BOTTOM + 1;
            player->v.z = 0;
            if (player->state & STATE_FLYING) player->state &= ~STATE_FLYING;
            player->state |= STATE_CAN_JUMP;
            player->state &= ~STATE_FALLING;
        }
        else player->state |= STATE_FALLING;
    }
    //TODO: move to new 'void parse_camera_collisions()'
    player->camera_distance = SETTING_CAMERA_DISTANCE_MAX;
}

f64 get_time_ms()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec + (f64)tp.tv_usec/1000000;
}

bool get_timer(f64 *time_start, f32 interval)
{
    if (get_time_ms() - *time_start >= interval)
    {
        *time_start = get_time_ms();
        return true;
    }
    return false;
}
