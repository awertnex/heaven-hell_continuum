#include <time.h>
#include <sys/time.h>

#include <engine/h/math.h>
#include "h/assets.h"
#include "h/chunking.h"
#include "h/main.h"
#include "h/logic.h"

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

void player_state_update(Render *render, Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v)
{
    /* ---- wrap coordinates ------------------------------------------------ */
    const i64 DIAMETER          = diameter * chunk_diameter;
    const i64 DIAMETER_V        = diameter_v * chunk_diameter;

    const i64 WORLD_MARGIN =
        (radius - SET_RENDER_DISTANCE_MAX) * chunk_diameter;
    const i64 WORLD_MARGIN_V =
        (radius_v - SET_RENDER_DISTANCE_MAX) * chunk_diameter;

    const i64 OVERFLOW_EDGE     = (radius + 1) * chunk_diameter;
    const i64 OVERFLOW_EDGE_V   = (radius_v + 1) * chunk_diameter;

    /* ---- wrap coordinates: world margin ---------------------------------- */
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

    /* ---- wrap coordinates: overflow edge --------------------------------- */
    if (player->pos.x > OVERFLOW_EDGE)      player->pos.x -= DIAMETER;
    if (player->pos.x < -OVERFLOW_EDGE)     player->pos.x += DIAMETER;
    if (player->pos.y > OVERFLOW_EDGE)      player->pos.y -= DIAMETER;
    if (player->pos.y < -OVERFLOW_EDGE)     player->pos.y += DIAMETER;
    if (player->pos.z > OVERFLOW_EDGE_V)    player->pos.z -= DIAMETER_V;
    if (player->pos.z < -OVERFLOW_EDGE_V)   player->pos.z += DIAMETER_V;

    player->chunk = (v3i16){
            floorf((f32)player->pos.x / chunk_diameter),
            floorf((f32)player->pos.y / chunk_diameter),
            floorf((f32)player->pos.z / chunk_diameter),
        };

    if ((player->delta_chunk.x - player->chunk.x)
            || (player->delta_chunk.y - player->chunk.y)
            || (player->delta_chunk.z - player->chunk.z))
        flag |= FLAG_MAIN_CHUNK_BUF_DIRTY;

    static f32 zoom = 0.0f;
    if (player->flag & FLAG_PLAYER_ZOOMER)
        zoom = player->camera.zoom;
    else zoom = 0.0f;

    if (player->flag & FLAG_PLAYER_FLYING)
    {
        player->flag &= ~FLAG_PLAYER_CAN_JUMP;
        player->camera.fovy = 80.0f;

        player->movement_speed = SET_PLAYER_SPEED_FLY;
        player->movement_lerp_speed.x = SET_LERP_SPEED_GLIDE;
        player->movement_lerp_speed.y = SET_LERP_SPEED_GLIDE;
        player->movement_lerp_speed.z = SET_LERP_SPEED_DEFAULT;
    }
    else gravity_update(render,
            &player->pos, &player->gravity_influence, player->mass);

    if ((player->flag & FLAG_PLAYER_SNEAKING)
            && !(player->flag & FLAG_PLAYER_FLYING))
        player->movement_speed = SET_PLAYER_SPEED_SNEAK;
    else if (player->flag & FLAG_PLAYER_SPRINTING)
    {
        if (!(player->flag & FLAG_PLAYER_FLYING))
        {
            player->movement_speed = SET_PLAYER_SPEED_SPRINT;
            player->camera.fovy = 75.0f;
        }
        else
        {
            player->movement_speed = SET_PLAYER_SPEED_FLY_FAST;
            player->camera.fovy = 90.0f;
        }
    }
    else if (!(player->flag & FLAG_PLAYER_SNEAKING)
            && !(player->flag & FLAG_PLAYER_SPRINTING)
            && !(player->flag & FLAG_PLAYER_FLYING))
    {
        player->movement_speed = SET_PLAYER_SPEED_WALK;
        player->movement_lerp_speed = (v3f32){
            SET_LERP_SPEED_DEFAULT,
            SET_LERP_SPEED_DEFAULT,
            SET_LERP_SPEED_RIGID,
        };

        player->camera.fovy = 70.0f;
    }

    player->camera.fovy -= zoom;
    player->camera.fovy =
        clamp_f32(player->camera.fovy, 1.0f, SET_FOV_MAX);
    player->camera.fovy_smooth =
        lerp_f32(player->camera.fovy_smooth,
                player->camera.fovy,
                settings.lerp_speed, render->frame_delta);

    player->camera_skybox.fovy = player->camera.fovy;
    player->camera_skybox.fovy_smooth = player->camera.fovy_smooth;
}

void player_camera_movement_update(Render *render, Player *player,
        b8 use_mouse)
{
    static f32 zoom = 0.0f;

    if (use_mouse)
    {
        if (player->flag & FLAG_PLAYER_ZOOMER)
            zoom = player->camera.zoom;
        else zoom = 0.0f;

        f32 sensitivity = settings.mouse_sensitivity /
            ((zoom / CAMERA_ZOOM_SENSITIVITY) + 1.0f);

        player->yaw += render->mouse_delta.x * sensitivity;
        player->pitch += render->mouse_delta.y * sensitivity;
    }

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
                    player->pos.x -
                        ((CYAW * CPCH) * player->camera_distance),
                    player->pos.y +
                        ((SYAW * CPCH) * player->camera_distance),
                    player->pos.z + player->eye_height +
                        (SPCH * player->camera_distance),
                };
            break;

        case MODE_CAMERA_3RD_PERSON_FRONT:
            player->camera.pos =
                (v3f32){
                    player->pos.x +
                        ((CYAW * CPCH) * player->camera_distance),
                    player->pos.y -
                        ((SYAW * CPCH) * player->camera_distance),
                    player->pos.z + player->eye_height -
                        (SPCH * player->camera_distance),
                };
            player->camera.sin_pitch = -SPCH;
            player->camera.sin_yaw =
                sin((player->yaw + (CAMERA_RANGE_MAX / 2.0f)) * DEG2RAD);
            player->camera.cos_yaw =
                cos((player->yaw + (CAMERA_RANGE_MAX / 2.0f)) * DEG2RAD);
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

    player->camera_skybox.sin_pitch =  player->camera.sin_pitch;
    player->camera_skybox.cos_pitch =  player->camera.cos_pitch;
    player->camera_skybox.sin_yaw =    player->camera.sin_yaw;
    player->camera_skybox.cos_yaw =    player->camera.cos_yaw;
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

    player->target_delta = (v3i64){
        (i64)floorf(player->target.x),
        (i64)floorf(player->target.y),
        (i64)floorf(player->target.z),
    };
}

void set_player_pos(Player *player, f64 x, f64 y, f64 z)
{
    player->pos = (v3f64){x, y, z};
    player->pos_last = player->pos;
}

void set_player_block(Player *player, i32 x, i32 y, i32 z)
{
    player->pos = (v3f64){(f64)x + 0.5f, (f64)y + 0.5f, (f64)z + 0.5f};
    player->pos_last = player->pos;
}

void player_kill(Player *player)
{
    player->vel = (v3f32){0};
    player->movement = (v3f32){0};
    player->movement_speed = 0.0f;
    player->container_state = 0;
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

void player_collision_update(Player *player)
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

    if (pos_last.x > pos.x)
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

    if (pos_last.y > pos.y)
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

    if (pos_last.z > pos.z)
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

    v3i32 aabb_pos =
    {
        (i32)player->collision_check_pos.x -
        (i32)player->chunk.x * CHUNK_DIAMETER,

        (i32)player->collision_check_pos.y -
        (i32)player->chunk.y * CHUNK_DIAMETER,

        (i32)player->collision_check_pos.z -
        (i32)player->chunk.z * CHUNK_DIAMETER,
    };

    v3i32 aabb_size =
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

    v3f64 box_2[2];
    Chunk *chunk = NULL;
    u32 *block = NULL;
    i32 x, y, z, dx, dy, dz, dcx, dcy, dcz;
    for (z = aabb_pos.z; z < aabb_pos.z + aabb_size.z; ++z)
    {
        dz = mod(z, CHUNK_DIAMETER);
        dcz = (i32)floorf((f32)z / CHUNK_DIAMETER);
        for (y = aabb_pos.y; y < aabb_pos.y + aabb_size.y; ++y)
        {
            dy = mod(y, CHUNK_DIAMETER);
            dcy = (i32)floorf((f32)y / CHUNK_DIAMETER);
            for (x = aabb_pos.x; x < aabb_pos.x + aabb_size.x; ++x)
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

                if (is_intersect_aabb(box_1, box_2))
                {
                    player->movement.z = 0.0f;
                    player->gravity_influence.z = 0.0f;
                    player->pos.z += box_2[1].z - box_1[0].z;
                    player->flag |= FLAG_PLAYER_CAN_JUMP;
                    player->flag &= ~FLAG_PLAYER_FLYING;

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

b8 is_intersect_aabb(v3f64 box_1[2], v3f64 box_2[2])
{
    return 
        (box_1[0].x <= box_2[1].x) && (box_1[1].x >= box_2[0].x) &&
        (box_1[0].y <= box_2[1].y) && (box_1[1].y >= box_2[0].y) &&
        (box_1[0].z <= box_2[1].z) && (box_1[1].z >= box_2[0].z);
};

void gravity_update(Render *render,
        v3f64 *position, v3f32 *influence, f32 mass)
{
    influence->z += (GRAVITY * mass * render->frame_delta);
    position->x += influence->x * render->frame_delta;
    position->y += influence->y * render->frame_delta;
    position->z += influence->z * render->frame_delta;
}
