#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

#include "engine/h/math.h"
#include "h/logic.h"
#include "h/chunking.h"
#include "h/settings.h"

void
update_player(Render *render, Player *player)
{
    player->chunk = (v3i16){
            floorf((f32)player->pos.x / CHUNK_DIAMETER),
            floorf((f32)player->pos.y / CHUNK_DIAMETER),
            floorf((f32)player->pos.z / CHUNK_DIAMETER),
        };

    if ((player->delta_chunk.x - player->chunk.x)
            || (player->delta_chunk.y - player->chunk.y)
            || (player->delta_chunk.z - player->chunk.z))
        state |= FLAG_CHUNK_BUF_DIRTY;

    if (!(player->state & FLAG_CAN_JUMP) &&
            !(player->state & FLAG_FLYING))
    {
        player->pos_lerp_speed.x =
            SETTING_LERP_SPEED_GLIDE;
        player->pos_lerp_speed.y =
            SETTING_LERP_SPEED_GLIDE;

        update_gravity(render, player);
    }

    if (player->state & FLAG_FLYING)
    {
        player->pos_lerp_speed.x =
            SETTING_LERP_SPEED_GLIDE;
        player->pos_lerp_speed.y =
            SETTING_LERP_SPEED_GLIDE;
        player->pos_lerp_speed.z =
            SETTING_LERP_SPEED_DEFAULT;

        player->vel = v3fzero;
        player->movement_speed =
            SETTING_PLAYER_SPEED_FLY * render->frame_delta;

        player->camera.fovy =
            lerp_f32(player->camera.fovy,
                    80.0f,
                    settings.lerp_speed, render->frame_delta);
    }

    if ((player->state & FLAG_SNEAKING)
            && !(player->state & FLAG_FLYING))
        player->movement_speed =
            SETTING_PLAYER_SPEED_SNEAK * render->frame_delta;
    else if (player->state & FLAG_SPRINTING)
    {
        if (!(player->state & FLAG_FLYING))
        {
            player->movement_speed =
                SETTING_PLAYER_SPEED_SPRINT * render->frame_delta;
            player->camera.fovy =
                lerp_f32(player->camera.fovy,
                        75.0f,
                        settings.lerp_speed, render->frame_delta);
        }
        else
        {
            player->movement_speed =
                SETTING_PLAYER_SPEED_FLY_FAST * render->frame_delta;
            player->camera.fovy =
                lerp_f32(player->camera.fovy,
                        90.0f,
                        settings.lerp_speed, render->frame_delta);
        }
    }
    else if (!(player->state & FLAG_SNEAKING)
            && !(player->state & FLAG_SPRINTING)
            && !(player->state & FLAG_FLYING))
    {
        player->pos_lerp_speed.x =
            SETTING_LERP_SPEED_DEFAULT;
        player->pos_lerp_speed.y =
            SETTING_LERP_SPEED_DEFAULT;
        player->pos_lerp_speed.z =
            SETTING_LERP_SPEED_RIGID;

        player->movement_speed =
            SETTING_PLAYER_SPEED_WALK * render->frame_delta;

        player->camera.fovy =
            lerp_f32(player->camera.fovy,
                    70.0f,
                        settings.lerp_speed, render->frame_delta);
    }
}

void
update_camera_movement_player(Render *render, Player *player)
{
    player->yaw += render->mouse_delta.x * settings.mouse_sensitivity;
    player->pitch += render->mouse_delta.y * settings.mouse_sensitivity;

    player->yaw = fmodf(player->yaw, CAMERA_RANGE_MAX);
    if (player->yaw < 0.0f)
        player->yaw += CAMERA_RANGE_MAX;

    player->pitch = clamp_f32(player->pitch,
            -CAMERA_ANGLE_MAX, CAMERA_ANGLE_MAX);

    player->sin_pitch = sin(player->pitch * DEG2RAD);
    player->cos_pitch = cos(player->pitch * DEG2RAD);
    player->sin_yaw =   sin(player->yaw * DEG2RAD);
    player->cos_yaw =   cos(player->yaw * DEG2RAD);
    const f32 SPCH = player->sin_pitch;
    const f32 CPCH = player->cos_pitch;
    const f32 SYAW = player->sin_yaw;
    const f32 CYAW = player->cos_yaw;
    player->camera.sin_pitch =  SPCH;
    player->camera.cos_pitch =  CPCH;
    player->camera.sin_yaw =    SYAW;
    player->camera.cos_yaw =    CYAW;

    switch (player->perspective)
    {
        case 0: /* ---- 1st person ------------------------------------------ */
            player->camera.pos =
                (v3f32){
                    player->pos.x, player->pos.y, player->pos.z +
                        player->eye_height
                };
            break;

        case 1: /* ---- 3rd person ------------------------------------------ */
            player->camera.pos =
                (v3f32){
                    player->pos.x - ((CYAW * CPCH) * player->camera_distance),
                    player->pos.y + ((SYAW * CPCH) * player->camera_distance),
                    player->pos.z +
                        player->eye_height + (SPCH * player->camera_distance),
                };
            break;

        case 2: /* ---- 3rd person opposite --------------------------------- */
            player->camera.pos =
                (v3f32){
                    player->pos.x + ((CYAW * CPCH) * player->camera_distance),
                    player->pos.y - ((SYAW * CPCH) * player->camera_distance),
                    player->pos.z +
                        player->eye_height + (SPCH * player->camera_distance),
                };
            player->camera.sin_yaw =
                sin((player->yaw + (CAMERA_RANGE_MAX / 2.0f)) * DEG2RAD);
            player->camera.cos_yaw =
                cos((player->yaw + (CAMERA_RANGE_MAX / 2.0f)) * DEG2RAD);
            break;

            /* TODO: make the stalker camera mode */
        case 3: /* ---- stalker --------------------------------------------- */
            break;

            /* TODO: make the spectator camera mode */
        case 4: /* ---- spectator ------------------------------------------- */
            break;
    }
}

void
update_player_target(v3f32 *player_target, v3i32 *player_delta_target)
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

void
player_kill(Player *player)
{
    player->vel = v3fzero;
    player->mass = 0.0f;
    player->movement_speed = 0.0f;
    player->container_state = 0;
    player->state = FLAG_DEAD;
}

void
player_respawn(Player *player)
{
    player->pos =
        (v3f32){
            player->spawn_point.x,
            player->spawn_point.y,
            player->spawn_point.z
        };
    player->state = 0;
}

/* TODO: make is_ray_intersect() */
b8
is_ray_intersect(Player *player)
{
    //if (target_chunk->i[player->delta_target.z][player->delta_target.y][player->delta_target.x])
        //return TRUE;
    return FALSE;
}

void
update_gravity(Render *render, Player *player)
{
    if (player->state & FLAG_FALLING)
        player->vel.z += (GRAVITY * player->mass * render->frame_delta);
    player->raw_pos.z += player->vel.z * render->frame_delta;
}

/* TODO: make AABB collision work */
void
update_collision_static(Player *player)
{
    player->collision_check_start = (v3f32){
            floorf(player->pos.x - (player->scl.x / 2.0f)) - 1.0f,
            floorf(player->pos.y - (player->scl.y / 2.0f)) - 1.0f,
            floorf(player->pos.z) - 1.0f,
        };

    player->collision_check_end = (v3f32){
            ceilf(player->scl.x) + 2.0f,
            ceilf(player->scl.y) + 2.0f,
            ceilf(player->scl.z) + 2.0f,
        };

    if (player->raw_pos.z < 0.0f)
    {
        if (player->state & FLAG_FLYING)
            player->state &= ~FLAG_FLYING;
        player->raw_pos.z = 0.0f;
        player->vel.z = 0.0f;
        player->state |= FLAG_CAN_JUMP;
        player->state &= ~FLAG_FALLING;
    }
    else if (!(player->state & FLAG_FLYING))
        player->state |= FLAG_FALLING;


#if 0 /* TODO: remove this parse collision feet stuff */
    Chunk *target_chunk = get_chunk(&player->lastPos, &player->state, FLAG_PARSE_COLLISION_FEET);
    if (target_chunk->i
            [z - 1 - WORLD_BOTTOM]
            [y - (target_chunk->pos.y * CHUNK_SIZE)]
            [x - (target_chunk->pos.x * CHUNK_SIZE)] & NOT_EMPTY)
    {
        player->pos.z = ceilf(targetCoordinatesFeet->z) + WORLD_BOTTOM + 1;
        player->vel.z = 0;
        if (player->state & FLAG_FLYING) player->state &= ~FLAG_FLYING;
        player->state |= FLAG_CAN_JUMP;
        player->state &= ~FLAG_FALLING;
    } else player->state |= FLAG_FALLING;

    // TODO: move to new 'void parse_camera_collisions()'
    player->camera_distance = SETTING_CAMERA_DISTANCE_MAX;
#endif
}

f64
get_time_ms()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec + (f64)tp.tv_usec / 1000000.0f;
}

b8
get_timer(f64 *time_start, f32 interval)
{
    if (get_time_ms() - *time_start >= interval)
    {
        *time_start = get_time_ms();
        return TRUE;
    }
    return FALSE;
}

#if 0 // TODO: undef
void
draw_default_grid(v4u8 x, v4u8 y, v4u8 z)
{
    v4u8 color = {0xff, 0xff, 0xff, 0xff};

    draw_line_3d((v3i32){-4, -4, 0}, (v3i32){4, -4, 0}, color);
    draw_line_3d((v3i32){-4, -3, 0}, (v3i32){4, -3, 0}, color);
    draw_line_3d((v3i32){-4, -2, 0}, (v3i32){4, -2, 0}, color);
    draw_line_3d((v3i32){-4, -1, 0}, (v3i32){4, -1, 0}, color);
    draw_line_3d((v3i32){-4, 0, 0}, (v3i32){4, 0, 0}, color);
    draw_line_3d((v3i32){-4, 1, 0}, (v3i32){4, 1, 0}, color);
    draw_line_3d((v3i32){-4, 2, 0}, (v3i32){4, 2, 0}, color);
    draw_line_3d((v3i32){-4, 3, 0}, (v3i32){4, 3, 0}, color);
    draw_line_3d((v3i32){-4, 4, 0}, (v3i32){4, 4, 0}, color);
    draw_line_3d((v3i32){-4, -4, 0}, (v3i32){-4, 4, 0}, color);
    draw_line_3d((v3i32){-3, -4, 0}, (v3i32){-3, 4, 0}, color);
    draw_line_3d((v3i32){-2, -4, 0}, (v3i32){-2, 4, 0}, color);
    draw_line_3d((v3i32){-1, -4, 0}, (v3i32){-1, 4, 0}, color);
    draw_line_3d((v3i32){0, -4, 0}, (v3i32){0, 4, 0}, color);
    draw_line_3d((v3i32){1, -4, 0}, (v3i32){1, 4, 0}, color);
    draw_line_3d((v3i32){2, -4, 0}, (v3i32){2, 4, 0}, color);
    draw_line_3d((v3i32){3, -4, 0}, (v3i32){3, 4, 0}, color);
    draw_line_3d((v3i32){4, -4, 0}, (v3i32){4, 4, 0}, color);

    draw_line_3d(v3izero, (v3i32){2, 0, 0}, x);
    draw_line_3d(v3izero, (v3i32){0, 2, 0}, y);
    draw_line_3d(v3izero, (v3i32){0, 0, 2}, z);
}
#endif // TODO: undef
