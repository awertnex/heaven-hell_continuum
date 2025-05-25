#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "h/logic.h"
#include "h/chunking.h"

// ---- variables --------------------------------------------------------------
Vector2 mouse_delta;

bool get_double_press(KeyboardKey key)
{
    static f64 double_press_start_time = 0;
    static KeyboardKey double_press_key = KEY_NULL;

    if (((state & FLAG_DOUBLE_PRESS) && (f64)(get_time_ms() - double_press_start_time >= 0.25f))
            || (key != double_press_key))
        state &= ~FLAG_DOUBLE_PRESS;
    else
    {
        double_press_key = KEY_NULL;
        return true;
    }

    if (!(state & FLAG_DOUBLE_PRESS))
    {
        state |= FLAG_DOUBLE_PRESS;
        double_press_key = key;
        double_press_start_time = get_time_ms();
    }
    return false;
}

void update_player(Player *player)
{
    player->chunk = (v2i16){
            floorf((f32)player->pos.x / CHUNK_DIAMETER),
            floorf((f32)player->pos.y / CHUNK_DIAMETER),
        };

    if ((lily.delta_chunk.x - lily.chunk.x)
            || (lily.delta_chunk.y - lily.chunk.y))
        state |= FLAG_CHUNK_BUF_DIRTY;

    if (!(player->state & FLAG_CAN_JUMP)
            && !(player->state & FLAG_FLYING))
        update_gravity(player);

    if (player->state & FLAG_FLYING)
    {
        player->v = v3fzero;
        player->movement_speed = PLAYER_SPEED_FLY * dt;
        player->camera.fovy = 80; //TODO: revise (add lerp)
    }

    if ((player->state & FLAG_SNEAKING)
            && !(player->state & FLAG_FLYING))
        player->movement_speed = PLAYER_SPEED_SNEAK * dt;
    else if (player->state & FLAG_SPRINTING)
    {
        if (!(player->state & FLAG_FLYING))
        {
            player->movement_speed = PLAYER_SPEED_SPRINT * dt;
            player->camera.fovy = 75;
        }
        else
        {
            player->movement_speed = PLAYER_SPEED_FLY_FAST * dt;
            player->camera.fovy = 90;
        }
    }
    else if (!(player->state & FLAG_SNEAKING)
            && !(player->state & FLAG_SPRINTING)
            && !(player->state & FLAG_FLYING))
    {
        player->movement_speed = PLAYER_SPEED_WALK * dt;
        player->camera.fovy = 70;
    }

    //TODO: revise (actually kill the player)
    if (player->pos.z < WORLD_KILL_Z)
    {
        set_player_block(player,
                player->spawn_point.x,
                player->spawn_point.y,
                player->spawn_point.z);
        player->v.z = 0;
    }
}

void update_camera_movements_player(Player *player)
{
    if (state & FLAG_PARSE_CURSOR)
        if (!(state_menu_depth) && !(state & FLAG_SUPER_DEBUG))
        {
            player->yaw -= (f32)mouse_delta.x * setting.mouse_sensitivity;
            player->pitch -= (f32)mouse_delta.y * setting.mouse_sensitivity;
        }

    player->yaw = fmod(player->yaw, 360.0f);
    if (player->yaw < 0.0f) player->yaw += 360.0f;
    if (player->pitch > 90.0f) player->pitch = 90.0f;
    else if (player->pitch < -90.0f) player->pitch = -90.0f;

    player->sin_pitch =  sin(player->pitch * MC_C_DEG2RAD);
    player->cos_pitch =  cos(player->pitch * MC_C_DEG2RAD);
    player->sin_yaw =    sin(player->yaw * MC_C_DEG2RAD);
    player->cos_yaw =    cos(player->yaw * MC_C_DEG2RAD);

    Vector3 player_camera_up =
    {
        -player->cos_yaw * player->sin_pitch,
        -player->sin_yaw * player->sin_pitch,
        player->cos_pitch,
    };

    switch (player->perspective)
    {
        case 0: // ---- 1st person ---------------------------------------------
            player->camera.position =
                (Vector3){player->pos.x, player->pos.y, player->pos.z + player->eye_height};
            player->camera.target =
                (Vector3){
                    player->pos.x + ((player->cos_yaw * player->cos_pitch) * setting.reach_distance),
                    player->pos.y + ((player->sin_yaw * player->cos_pitch) * setting.reach_distance),
                    player->camera.position.z + (player->sin_pitch * setting.reach_distance),
                };
            player->camera.up = player_camera_up;
            break;

        case 1: // ---- 3rd person back ----------------------------------------
            player->camera.position =
                (Vector3){
                    player->pos.x - ((player->cos_yaw * player->cos_pitch) * player->camera_distance),
                    player->pos.y - ((player->sin_yaw * player->cos_pitch) * player->camera_distance),
                    player->pos.z + player->eye_height - (player->sin_pitch * player->camera_distance),
                };
            player->camera.target =
                (Vector3){player->pos.x, player->pos.y, player->pos.z + player->eye_height};
            player->camera.up = player_camera_up;
            break;

        case 2: // ---- 3rd person front ---------------------------------------
            player->camera.position =
                (Vector3){
                    player->pos.x + ((player->cos_yaw * player->cos_pitch) * player->camera_distance),
                    player->pos.y + ((player->sin_yaw * player->cos_pitch) * player->camera_distance),
                    player->pos.z + player->eye_height + (player->sin_pitch * player->camera_distance),
                };
            player->camera.target =
                (Vector3){player->pos.x, player->pos.y, player->pos.z + player->eye_height};
            player->camera.up = player_camera_up;
            break;

            // TODO: make the stalker camera mode
        case 3: // ---- stalker ------------------------------------------------
            player->camera.target =
                (Vector3){player->pos.x, player->pos.y, player->pos.z + player->eye_height};
            break;

            // TODO: make the spectator camera mode
        case 4: // ---- spectator ----------------------------------------------
            break;
    }

    // ---- camera_debug_mode --------------------------------------------------
    if (!(state & FLAG_DEBUG)) return;
    switch (player->perspective)
    {
        case 0: // ---- 1st person ---------------------------------------------
        case 1: // ---- 3rd person back ----------------------------------------
            player->camera_debug_info.position =
                (Vector3){
                    -player->cos_yaw * player->cos_pitch,
                    -player->sin_yaw * player->cos_pitch,
                    -player->sin_pitch,
                };
            player->camera_debug_info.target = Vector3Zero();
            player->camera_debug_info.up = player_camera_up;
            break;
        case 2: // ---- 3rd person front ---------------------------------------
            player->camera_debug_info.position =
                (Vector3){
                    player->cos_yaw * player->cos_pitch,
                    player->sin_yaw * player->cos_pitch,
                    player->sin_pitch,
                };
            player->camera_debug_info.target = Vector3Zero();
            player->camera_debug_info.up = player_camera_up;
            break;

            // TODO: make the stalker camera mode
        case 3: // ---- stalker ------------------------------------------------
            break;

            // TODO: make the spectator camera mode
        case 4: // ---- spectator ----------------------------------------------
            break;
    }
}

void update_player_target(Vector3 *player_target, v3i32 *player_delta_target)
{
    if ((i32)player_delta_target->x != floorf(player_target->x)
            || (i32)player_delta_target->y != floorf(player_target->y)
            || (i32)player_delta_target->z != floorf(player_target->z))
        *player_delta_target =
            (v3i32){
                (i32)floorf(player_target->x),
                (i32)floorf(player_target->y),
                (i32)floorf(player_target->z)};
}

void set_player_pos(Player *player, f32 x, f32 y, f32 z)
{
    player->pos = (Vector3){x, y, z};
}

void set_player_block(Player *player, i32 x, i32 y, i32 z)
{
    player->pos = (Vector3){(f32)(x + 0.5f), (f32)(y + 0.5f), (f32)(z + 0.5f)};
}

void kill_player(Player *player)
{
    player->v = v3fzero;
    player->m = 0.0f;
    player->movement_speed = 0.0f;
    player->container_state = 0;
    player->state = FLAG_DEAD;
}

void player_respawn(Player *player)
{
    player->pos =
        (Vector3){
            player->spawn_point.x,
            player->spawn_point.y,
            player->spawn_point.z
        };
    player->state = 0;
}

u32 get_distance(v2i32 a, v2i32 b)
{
    return powf(a.x - b.x, 2) + powf(a.y - b.y, 2);
}

b8 is_range_within_i(i32 pos, i32 start, i32 end)
{
    if (pos < start || pos > end)
        return FALSE;
    return TRUE;
}

b8 is_range_within_f(f32 pos, f32 start, f32 end)
{
    if (pos < start || pos > end)
        return FALSE;
    return TRUE;
}

b8 is_range_within_v2f(v2f32 pos, v2f32 start, v2f32 end)
{
    if (pos.x < start.x || pos.x > end.x
            || pos.y < start.y || pos.y > end.y)
        return FALSE;
    return TRUE;
}

b8 is_range_within_v3i(v3i32 pos, v3i32 start, v3i32 end)
{
    if (pos.x < start.x || pos.x > end.x
            || pos.y < start.y || pos.y > end.y
            || pos.z < start.z || pos.z > end.z)
        return FALSE;
    return TRUE;
}

b8 is_range_within_v3fi(v3f32 pos, v3i32 start, v3i32 end)
{
    if ((i32)pos.x < start.x || (i32)pos.x > end.x
            || (i32)pos.y < start.y || (i32)pos.y > end.y
            || (i32)pos.z < start.z || (i32)pos.z > end.z)
        return FALSE;
    return TRUE;
}

b8 is_ray_intersect(Player *player) //TODO: make the player ray intersection
{
    //if (target_chunk->i[player->delta_target.z][player->delta_target.y][player->delta_target.x])
        //return true;
    return false;
}

void update_gravity(Player *player) // TODO: fix player gravity
{
    if (player->state & FLAG_FALLING)
        player->v.z -= (PLAYER_JUMP_HEIGHT * GRAVITY * player->m * dt);
    player->pos.z += player->v.z;
    //printf("   previous time: %.2lf   delta time: %.2lf\n", get_time_ms(), get_delta_time(&start_time)); //temp
}

/*
void update_collision_static(player *player) // TODO: make AABB collision work
{
    player->collision_check_start = (Vector3){
            floorf(player->pos.x - (player->scl.x / 2.0f)) - 1,
            floorf(player->pos.y - (player->scl.y / 2.0f)) - 1,
            floorf(player->pos.z) - 1,
        };

    player->collision_check_end = (Vector3){
            ceilf(player->scl.x) + 2,
            ceilf(player->scl.y) + 2,
            ceilf(player->scl.z) + 2,
        };

    Chunk *target_chunk = get_chunk(&player->lastPos, &player->state, FLAG_PARSE_COLLISION_FEET);
    if ((player->state & FLAG_PARSE_COLLISION_FEET)
        && player->pos.z > WORLD_BOTTOM)
    {
        if (target_chunk->i
                [z - 1 - WORLD_BOTTOM]
                [y - (target_chunk->pos.y * CHUNK_SIZE)]
                [x - (target_chunk->pos.x * CHUNK_SIZE)] & NOT_EMPTY)
        {
            player->pos.z = ceilf(targetCoordinatesFeet->z) + WORLD_BOTTOM + 1;
            player->v.z = 0;
            if (player->state & FLAG_FLYING) player->state &= ~FLAG_FLYING;
            player->state |= FLAG_CAN_JUMP;
            player->state &= ~FLAG_FALLING;
        } else player->state |= FLAG_FALLING;
    }
    //TODO: move to new 'void parse_camera_collisions()'
    player->camera_distance = SETTING_CAMERA_DISTANCE_MAX;
}
*/

f64 get_time_ms()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec + (f64)tp.tv_usec / 1000000.0f;
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

