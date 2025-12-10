#include <time.h>
#include <sys/time.h>

#include <engine/h/math.h>
#include "h/assets.h"
#include "h/chunking.h"
#include "h/main.h"
#include "h/logic.h"

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief handle player being near or past world edges.
 *
 *  teleport player to the other side of the world if they cross a world edge.
 *
 *  @param radius = world radius.
 *  @param radius_v = world radius, vertical.
 *  @param diameter = world diameter.
 *  @param diameter_v = world diameter, vertical.
 */
static void player_wrap_coordinates(Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v);

f64 get_time_ms(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (f64)tp.tv_sec + (f64)tp.tv_usec * 1e-6f;
}

b8 get_timer(f64 *time_start, f32 interval)
{
    f64 time_current = get_time_ms();
    if (time_current - *time_start >= interval)
    {
        *time_start = time_current;
        return TRUE;
    }
    return FALSE;
}

void player_state_update(f64 dt, Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v)
{
    /* ---- setup parameters ------------------------------------------------ */

    player_wrap_coordinates(player, chunk_diameter,
            radius, radius_v, diameter, diameter_v);

    player->chunk = (v3i16){
            floorf((f32)player->pos.x / chunk_diameter),
            floorf((f32)player->pos.y / chunk_diameter),
            floorf((f32)player->pos.z / chunk_diameter),
        };

    if (
            player->chunk_delta.x - player->chunk.x ||
            player->chunk_delta.y - player->chunk.y ||
            player->chunk_delta.z - player->chunk.z)
        flag |= FLAG_MAIN_CHUNK_BUF_DIRTY;

    player->movement_speed = SET_PLAYER_SPEED_WALK;
    player->movement_lerp_speed = (v3f32){
        SET_LERP_SPEED_WALK,
        SET_LERP_SPEED_WALK,
        SET_LERP_SPEED_WALK,
    };
    player->camera.fovy = settings.fov;

    /* ---- player flags ---------------------------------------------------- */

    if (player->flag & FLAG_PLAYER_FLYING)
    {
        player->flag &= ~FLAG_PLAYER_CAN_JUMP;
        player->camera.fovy += 10.0f;

        player->movement_speed = SET_PLAYER_SPEED_FLY;
        player->movement_lerp_speed = (v3f32){
            SET_LERP_SPEED_GLIDE,
            SET_LERP_SPEED_GLIDE,
            SET_LERP_SPEED_GLIDE_V,
        };

        if (player->flag & FLAG_PLAYER_SPRINTING)
        {
            player->movement_speed = SET_PLAYER_SPEED_FLY_FAST;
            player->camera.fovy += 10.0f;
        }
    }
    else
    {
        gravity_update(dt, &player->pos, &player->gravity_influence, player->weight);

        if (player->flag & FLAG_PLAYER_SNEAKING)
            player->movement_speed = SET_PLAYER_SPEED_SNEAK;
        else if (player->flag & FLAG_PLAYER_SPRINTING)
        {
            player->movement_speed = SET_PLAYER_SPEED_SPRINT;
            player->camera.fovy += 5.0f;
        }
    }

    if (player->flag & FLAG_PLAYER_ZOOMER && player->camera.zoom)
        player->camera.fovy = settings.fov - player->camera.zoom;

    /* ---- post-process parameters ----------------------------------------- */

    player->movement_smooth = (v3f32){
        lerp_f32(player->movement_smooth.x, player->movement.x,
                dt, player->movement_lerp_speed.x),

        lerp_f32(player->movement_smooth.y, player->movement.y,
                dt, player->movement_lerp_speed.y),

        lerp_f32(player->movement_smooth.z, player->movement.z,
                dt, player->movement_lerp_speed.z),
    };

    player->pos = (v3f64){
        player->pos.x + player->movement_smooth.x * dt,
        player->pos.y + player->movement_smooth.y * dt,
        player->pos.z + player->movement_smooth.z * dt,
    };

    player->velocity = (v3f32){
        (player->pos.x - player->pos_last.x) / dt,
        (player->pos.y - player->pos_last.y) / dt,
        (player->pos.z - player->pos_last.z) / dt,
    };

    player->pos_last = player->pos;
    player->speed = sqrtf(len_v3f32(player->velocity));
    if (player->speed > EPSILON)
        player->camera.fovy += player->speed * 0.03f;
    player->camera.fovy = clamp_f32(player->camera.fovy, 1.0f, SET_FOV_MAX);
    player->camera.fovy_smooth =
        lerp_f32(player->camera.fovy_smooth, player->camera.fovy,
                dt, SET_LERP_SPEED_FOV_MODE);
}

static void player_wrap_coordinates(Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v)
{
    /* ---- setup parameters ------------------------------------------------ */

    const i64 DIAMETER = diameter * chunk_diameter;
    const i64 DIAMETER_V = diameter_v * chunk_diameter;

    const i64 WORLD_MARGIN =
        (radius - SET_RENDER_DISTANCE_MAX) * chunk_diameter;
    const i64 WORLD_MARGIN_V =
        (radius_v - SET_RENDER_DISTANCE_MAX) * chunk_diameter;

    const i64 OVERFLOW_EDGE =     (radius + 1) * chunk_diameter;
    const i64 OVERFLOW_EDGE_V =   (radius_v + 1) * chunk_diameter;

    /* ---- world margin ---------------------------------------------------- */

    if (player->pos.x > WORLD_MARGIN)
        player->flag |= FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX;
    else if (player->pos.x < -WORLD_MARGIN)
    {
        player->flag |= FLAG_PLAYER_OVERFLOW_X;
        player->flag &= ~FLAG_PLAYER_OVERFLOW_PX;
    }
    else player->flag &= ~(FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX);

    if (player->pos.y > WORLD_MARGIN)
        player->flag |= FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY;
    else if (player->pos.y < -WORLD_MARGIN)
    {
        player->flag |= FLAG_PLAYER_OVERFLOW_Y;
        player->flag &= ~FLAG_PLAYER_OVERFLOW_PY;
    }
    else player->flag &= ~(FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY);

    if (player->pos.z > WORLD_MARGIN_V)
        player->flag |= FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ;
    else if (player->pos.z < -WORLD_MARGIN_V)
    {
        player->flag |= FLAG_PLAYER_OVERFLOW_Z;
        player->flag &= ~FLAG_PLAYER_OVERFLOW_PZ;
    }
    else player->flag &= ~(FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ);

    /* ---- overflow edge --------------------------------------------------- */

    if (player->pos.x > OVERFLOW_EDGE)
    {
        player->pos.x -= DIAMETER;
        player->pos_last.x -= DIAMETER;
    }
    if (player->pos.x < -OVERFLOW_EDGE)
    {
        player->pos.x += DIAMETER;
        player->pos_last.x += DIAMETER;
    }
    if (player->pos.y > OVERFLOW_EDGE)
    {
        player->pos.y -= DIAMETER;
        player->pos_last.y -= DIAMETER;
    }
    if (player->pos.y < -OVERFLOW_EDGE)
    {
        player->pos.y += DIAMETER;
        player->pos_last.y += DIAMETER;
    }
    if (player->pos.z > OVERFLOW_EDGE_V)
    {
        player->pos.z -= DIAMETER_V;
        player->pos_last.z -= DIAMETER_V;
    }
    if (player->pos.z < -OVERFLOW_EDGE_V)
    {
        player->pos.z += DIAMETER_V;
        player->pos_last.z += DIAMETER_V;
    }
}

void player_camera_movement_update(
        v2f64 mouse_delta, Player *player, b8 use_mouse)
{
    static f32 zoom = 0.0f;

    if (use_mouse)
    {
        if (player->flag & FLAG_PLAYER_ZOOMER)
            zoom = player->camera.zoom;
        else zoom = 0.0f;

        f32 sensitivity = settings.mouse_sensitivity /
            (zoom / CAMERA_ZOOM_SENSITIVITY + 1.0f);

        player->yaw += mouse_delta.x * sensitivity;
        player->pitch += mouse_delta.y * sensitivity;
    }

    player->yaw = fmodf(player->yaw, CAMERA_RANGE_MAX);
    if (player->yaw < 0.0f)
        player->yaw += CAMERA_RANGE_MAX;

    player->pitch = clamp_f32(player->pitch, -CAMERA_ANGLE_MAX, CAMERA_ANGLE_MAX);

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

    switch (player->camera_mode)
    {
        case MODE_CAMERA_1ST_PERSON:
            player->camera.pos =
                (v3f32){
                    player->pos.x,
                    player->pos.y,
                    player->pos.z + player->eye_height
                };
            break;

        case MODE_CAMERA_3RD_PERSON:
            player->camera.pos =
                (v3f32){
                    player->pos.x - CYAW * CPCH * player->camera_distance,
                    player->pos.y + SYAW * CPCH * player->camera_distance,
                    player->pos.z + player->eye_height + SPCH * player->camera_distance,
                };
            break;

        case MODE_CAMERA_3RD_PERSON_FRONT:
            player->camera.pos =
                (v3f32){
                    player->pos.x + CYAW * CPCH * player->camera_distance,
                    player->pos.y - SYAW * CPCH * player->camera_distance,
                    player->pos.z + player->eye_height - SPCH * player->camera_distance,
                };
            player->camera.sin_pitch = -SPCH;
            player->camera.sin_yaw = sin((player->yaw + CAMERA_RANGE_MAX / 2.0f) * DEG2RAD);
            player->camera.cos_yaw = cos((player->yaw + CAMERA_RANGE_MAX / 2.0f) * DEG2RAD);
            break;

            /* TODO: make the stalker camera mode */
        case MODE_CAMERA_STALKER:
            break;

            /* TODO: make the spectator camera mode */
        case MODE_CAMERA_SPECTATOR:
            break;
    }

    player->camera_hud.sin_pitch =  player->camera.sin_pitch;
    player->camera_hud.cos_pitch =  player->camera.cos_pitch;
    player->camera_hud.sin_yaw =    player->camera.sin_yaw;
    player->camera_hud.cos_yaw =    player->camera.cos_yaw;
}

void player_target_update(Player *player)
{
    const f32 SPCH = player->sin_pitch;
    const f32 CPCH = player->cos_pitch;
    const f32 SYAW = player->sin_yaw;
    const f32 CYAW = player->cos_yaw;

    player->target = (v3f64){
        player->pos.x + CYAW * CPCH * settings.reach_distance,
        player->pos.y - SYAW * CPCH * settings.reach_distance,
        player->pos.z + player->eye_height - SPCH * settings.reach_distance,
    };

    player->target_snapped = (v3i64){
        (i64)floor(player->target.x),
        (i64)floor(player->target.y),
        (i64)floor(player->target.z),
    };
}

void set_player_pos(Player *player, f64 x, f64 y, f64 z)
{
    player->pos = (v3f64){x, y, z};
    player->pos_last = player->pos;
}

void set_player_block(Player *player, i64 x, i64 y, i64 z)
{
    player->pos = (v3f64){(f64)x + 0.5f, (f64)y + 0.5f, (f64)z + 0.5f};
    player->pos_last = player->pos;
}

void set_player_spawn(Player *player, i64 x, i64 y, i64 z)
{
    player->spawn_point = (v3i64){x, y, z};
}

void player_kill(Player *player)
{
    player->movement = (v3f32){0};
    player->velocity = (v3f32){0};
    player->gravity_influence = (v3f32){0};
    player->flag |= FLAG_PLAYER_DEAD;
}

void player_respawn(Player *player)
{
    player->pos = (v3f64){
        player->spawn_point.x,
        player->spawn_point.y,
        player->spawn_point.z
    };
    player->flag = 0;
}

void player_collision_update(f64 dt, Player *player)
{
    if (!MODE_INTERNAL_COLLIDE) return;

    v3f64 pos = player->pos;
    v3f64 pos_last = player->pos_last;
    v3f64 pos_delta =
    {
        pos.x - pos_last.x,
        pos.y - pos_last.y,
        pos.z - pos_last.z,
    };
    v3f64 check_pos = {0};
    v3f64 check_size = {0};
    v3f64 check_delta = {0};

    if (pos.x < pos_last.x)
    {
        check_pos.x = pos.x + pos_delta.x - (player->size.x / 2.0f) - 1.0f;
        check_delta.x = check_pos.x - ceil(check_pos.x);
        check_size.x = player->size.x - pos_delta.x + 3.0f + check_delta.x;
    }
    else
    {
        check_pos.x = pos_last.x + pos_delta.x - (player->size.x / 2.0) - 1.0f;
        check_delta.x = check_pos.x - ceil(check_pos.x);
        check_size.x = player->size.x + pos_delta.x + 3.0f + check_delta.x;
    }

    if (pos.y < pos_last.y)
    {
        check_pos.y = pos.y + pos_delta.y - (player->size.y / 2.0f) - 1.0f;
        check_delta.y = check_pos.y - ceil(check_pos.y);
        check_size.y = player->size.y - pos_delta.y + 3.0f + check_delta.y;
    }
    else
    {
        check_pos.y = pos_last.y + pos_delta.y - (player->size.y / 2.0f) - 1.0f;
        check_delta.y = check_pos.y - ceil(check_pos.y);
        check_size.y = player->size.y + pos_delta.y + 3.0f + check_delta.y;
    }

    if (pos.z < pos_last.z)
    {
        check_pos.z = pos.z + pos_delta.z - 1.0f;
        check_delta.z = check_pos.z - ceil(check_pos.z);
        check_size.z = player->size.z - pos_delta.z + 3.0f + check_delta.z;
    }
    else
    {
        check_pos.z = pos_last.z + pos_delta.z - 1.0f;
        check_delta.z = check_pos.z - ceil(check_pos.z);
        check_size.z = player->size.z + pos_delta.z + 3.0f + check_delta.z;
    }

    player->collision_check_pos = (v3f64){
        floor(check_pos.x),
        floor(check_pos.y),
        floor(check_pos.z),
    };

    player->collision_check_size = (v3f64){
        ceil(check_size.x),
        ceil(check_size.y),
        ceil(check_size.z),
    };

    v3i32 check_pos_snapped =
    {
        (i32)player->collision_check_pos.x -
        (i32)player->chunk.x * CHUNK_DIAMETER,

        (i32)player->collision_check_pos.y -
        (i32)player->chunk.y * CHUNK_DIAMETER,

        (i32)player->collision_check_pos.z -
        (i32)player->chunk.z * CHUNK_DIAMETER,
    };

    v3i32 check_size_snapped =
    {
        (i32)player->collision_check_size.x,
        (i32)player->collision_check_size.y,
        (i32)player->collision_check_size.z,
    };

    v3f64 box_1[2] =
    {
        {
            player->pos.x - player->size.x / 2.0f,
            player->pos.y - player->size.y / 2.0f,
            player->pos.z,
        },
        {
            player->pos.x + player->size.x / 2.0f,
            player->pos.y + player->size.y / 2.0f,
            player->pos.z + player->size.z,
        },
    };

    v3f64 box_1_last[2] =
    {
        {
            player->pos_last.x - player->size.x / 2.0f,
            player->pos_last.y - player->size.y / 2.0f,
            player->pos_last.z,
        },
        {
            player->pos_last.x + player->size.x / 2.0f,
            player->pos_last.y + player->size.y / 2.0f,
            player->pos_last.z + player->size.z,
        },
    };

    v3f64 box_2[2];
    Chunk *chunk = NULL;
    u32 *block = NULL;
    i32 x, y, z, dx, dy, dz, dcx, dcy, dcz;
    for (z = check_pos_snapped.z;
            z < check_pos_snapped.z + check_size_snapped.z; ++z)
    {
        dz = mod(z, CHUNK_DIAMETER);
        dcz = (i32)floorf((f32)z / CHUNK_DIAMETER);
        for (y = check_pos_snapped.y;
                y < check_pos_snapped.y + check_size_snapped.y; ++y)
        {
            dy = mod(y, CHUNK_DIAMETER);
            dcy = (i32)floorf((f32)y / CHUNK_DIAMETER);
            for (x = check_pos_snapped.x;
                    x < check_pos_snapped.x + check_size_snapped.x; ++x)
            {
                dx = mod(x, CHUNK_DIAMETER);
                dcx = (i32)floorf((f32)x / CHUNK_DIAMETER);

                u64 index = settings.chunk_tab_center + dcx +
                    dcy * settings.chunk_buf_diameter +
                    dcz * settings.chunk_buf_layer;
                chunk = chunk_tab[index];
                if (!chunk) continue;
                block = &chunk->block[dz][dy][dx];
                if (!block || !*block) continue;

                box_2[0] = (v3f64){
                    (f64)((i64)chunk->pos.x * CHUNK_DIAMETER + dx),
                    (f64)((i64)chunk->pos.y * CHUNK_DIAMETER + dy),
                    (f64)((i64)chunk->pos.z * CHUNK_DIAMETER + dz),
                };
                box_2[1] = (v3f64){
                    box_2[0].x + 1.0f,
                    box_2[0].y + 1.0f,
                    box_2[0].z + 1.0f,
                };

                if (is_intersect_aabb(box_1, box_1_last, box_2))
                {
                    v3f32 diff_negative =
                    {
                        box_2[1].x - box_1[0].x,
                        box_2[1].y - box_1[0].y,
                        box_2[1].z - box_1[0].z,
                    };
                    v3f32 diff_positive =
                    {
                        box_2[0].x - box_1[1].x,
                        box_2[0].y - box_1[1].y,
                        box_2[0].z - box_1[1].z,
                    };

                    if (box_1[0].x > box_2[0].x)
                    {
                        //player->pos.x += diff_negative.x;
                        player->movement_smooth.x = 0.0f;
                    }
                    if (box_1[1].x < box_2[1].x)
                    {
                        //player->pos.x += diff_positive.x;
                        player->movement_smooth.x = 0.0f;
                    }
                    if (box_1[0].y > box_2[0].y)
                    {
                        //player->pos.y += diff_negative.y;
                        player->movement_smooth.y = 0.0f;
                    }
                    if (box_1[1].y < box_2[1].y)
                    {
                        //player->pos.y += diff_positive.y;
                        player->movement_smooth.y = 0.0f;
                    }
                    if (box_1[0].z > box_2[0].z)
                    {
                        //player->pos.z += diff_negative.z;
                        player->movement_smooth.z = 0.0f;
                        player->gravity_influence.z = 0.0f;
                        player->flag |= FLAG_PLAYER_CAN_JUMP;
                        player->flag &= ~FLAG_PLAYER_FLYING;
                    }
                    if (box_1[1].z < box_2[1].z)
                    {
                        //player->pos.z += diff_positive.z;
                        player->movement_smooth.z = 0.0f;
                        player->gravity_influence.z = 0.0f;
                    }

                    player->pos.x -= player->velocity.x * dt;
                    player->pos.y -= player->velocity.y * dt;
                    player->pos.z -= player->velocity.z * dt;

                    box_1[0] = (v3f64){
                        player->pos.x - (player->size.x / 2.0f),
                        player->pos.y - (player->size.y / 2.0f),
                        player->pos.z,
                    };
                    box_1[1] = (v3f64){
                        player->pos.x + (player->size.x / 2.0f),
                        player->pos.y + (player->size.y / 2.0f),
                        player->pos.z + player->size.z,
                    };
                }
            }
        }
    }
}

b8 is_intersect_aabb(v3f64 box_1[2], v3f64 box_1_last[2], v3f64 box_2[2])
{
    v3u8 is_intersect =
    {
        (box_1[0].x <= box_2[1].x) && (box_1[1].x >= box_2[0].x),

        (box_1[0].y <= box_2[1].y) && (box_1[1].y >= box_2[0].y),

        (box_1[0].z <= box_2[1].z) && (box_1[1].z >= box_2[0].z),
    };

    return is_intersect.x && is_intersect.y && is_intersect.z;
};

void gravity_update(f64 dt, v3f64 *position, v3f32 *acceleration, f32 weight)
{
    acceleration->z += GRAVITY * weight * dt;
    position->x += acceleration->x * dt;
    position->y += acceleration->y * dt;
    position->z += acceleration->z * dt;
}
