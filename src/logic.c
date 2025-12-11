#include <time.h>
#include <sys/time.h>

#include <engine/h/math.h>

#include "h/main.h"
#include "h/assets.h"
#include "h/chunking.h"
#include "h/logic.h"

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief handle player being near or past world edges.
 *
 *  teleport player to the other side of the world if they cross a world edge.
 */
static void player_wrap_coordinates(Player *p);

u64 get_time_logic(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (u64)(tp.tv_sec + tp.tv_usec);
}

f64 get_time_f64(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (f64)tp.tv_sec + (f64)tp.tv_usec * 1e-6f;
}

b8 get_timer(f64 *time_start, f32 interval)
{
    f64 time_current = get_time_f64();
    if (time_current - *time_start >= interval)
    {
        *time_start = time_current;
        return TRUE;
    }
    return FALSE;
}

void player_state_update(Player *p, f64 dt)
{
    p->movement_speed = SET_PLAYER_SPEED_WALK;
    p->movement_lerp_speed = (v3f32){
        SET_LERP_SPEED_WALK,
        SET_LERP_SPEED_WALK,
        SET_LERP_SPEED_WALK,
    };
    p->camera.fovy = settings.fov;

    /* ---- player flags ---------------------------------------------------- */

    if (p->flag & FLAG_PLAYER_FLYING)
    {
        p->flag &= ~FLAG_PLAYER_CAN_JUMP;
        p->flag &= ~FLAG_PLAYER_MID_AIR;

        p->movement_speed = SET_PLAYER_SPEED_FLY;
        p->movement_lerp_speed = (v3f32){
            SET_LERP_SPEED_GLIDE,
            SET_LERP_SPEED_GLIDE,
            SET_LERP_SPEED_GLIDE_V,
        };

        p->gravity_influence.z = 0.0f;

        p->camera.fovy += 10.0f;

        if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->movement_speed = SET_PLAYER_SPEED_FLY_FAST;
            p->camera.fovy += 10.0f;
        }
    }
    else
    {
        gravity_update(&p->pos, &p->gravity_influence, p->weight, dt);

        if (!(p->flag & FLAG_PLAYER_CAN_JUMP))
            p->flag |= FLAG_PLAYER_MID_AIR;

        if (p->flag & FLAG_PLAYER_SNEAKING)
            p->movement_speed = SET_PLAYER_SPEED_SNEAK;
        else if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->movement_speed = SET_PLAYER_SPEED_SPRINT;
            p->camera.fovy += 5.0f;
        }
    }

    if (p->flag & FLAG_PLAYER_ZOOMER && p->camera.zoom)
        p->camera.fovy = settings.fov - p->camera.zoom;

    /* ---- post-process parameters ----------------------------------------- */

    p->movement_smooth = (v3f32){
        lerp_f32(p->movement_smooth.x, p->movement.x, dt, p->movement_lerp_speed.x),
        lerp_f32(p->movement_smooth.y, p->movement.y, dt, p->movement_lerp_speed.y),
        lerp_f32(p->movement_smooth.z, p->movement.z, dt, p->movement_lerp_speed.z),
    };

    p->pos = (v3f64){
        p->pos.x + p->movement_smooth.x * dt,
        p->pos.y + p->movement_smooth.y * dt,
        p->pos.z + p->movement_smooth.z * dt,
    };

    p->velocity = (v3f32){
        (p->pos.x - p->pos_last.x) / dt,
        (p->pos.y - p->pos_last.y) / dt,
        (p->pos.z - p->pos_last.z) / dt,
    };

    p->pos_last = p->pos;
    p->speed = sqrtf(len_v3f32(p->velocity));
    if (p->speed > EPSILON)
        p->camera.fovy += p->speed * 0.03f;
    p->camera.fovy = clamp_f32(p->camera.fovy, 1.0f, SET_FOV_MAX);
    p->camera.fovy_smooth = lerp_f32(p->camera.fovy_smooth, p->camera.fovy,
                dt, SET_LERP_SPEED_FOV_MODE);

    /* ---- collision and correction ---------------------------------------- */

    //for (u32 i = 0; i < 400000; ++i)
    player_wrap_coordinates(p);
    player_chunk_update(p);
    player_collision_update(p, dt);
}

void player_chunk_update(Player *p)
{
    p->chunk = (v3i32){
        floorf((f32)p->pos.x / CHUNK_DIAMETER),
        floorf((f32)p->pos.y / CHUNK_DIAMETER),
        floorf((f32)p->pos.z / CHUNK_DIAMETER),
    };

    if (
            p->chunk_delta.x - p->chunk.x ||
            p->chunk_delta.y - p->chunk.y ||
            p->chunk_delta.z - p->chunk.z)
        flag |= FLAG_MAIN_CHUNK_BUF_DIRTY;
}

static void player_wrap_coordinates(Player *p)
{
    i64 DIAMETER = WORLD_DIAMETER * CHUNK_DIAMETER,
        DIAMETER_V = WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER,
        WORLD_MARGIN = (WORLD_RADIUS - SET_RENDER_DISTANCE_MAX) * CHUNK_DIAMETER,
        WORLD_MARGIN_V = (WORLD_RADIUS_VERTICAL - SET_RENDER_DISTANCE_MAX) * CHUNK_DIAMETER,
        OVERFLOW_EDGE = (WORLD_RADIUS + 1) * CHUNK_DIAMETER,
        OVERFLOW_EDGE_V = (WORLD_RADIUS_VERTICAL + 1) * CHUNK_DIAMETER;

    /* ---- safety reset ---------------------------------------------------- */

    if (!is_in_volume_f64(p->pos,
                (v3f64){
                -(f64)(WORLD_DIAMETER + WORLD_RADIUS),
                -(f64)(WORLD_DIAMETER + WORLD_RADIUS),
                -(f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL)},
                (v3f64){
                (f64)(WORLD_DIAMETER + WORLD_RADIUS),
                (f64)(WORLD_DIAMETER + WORLD_RADIUS),
                (f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL)}))
        player_spawn(p, FALSE);

    /* ---- world margin ---------------------------------------------------- */

    if (p->pos.x > WORLD_MARGIN)
        p->flag |= FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX;
    else if (p->pos.x < -WORLD_MARGIN)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_X;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PX;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX);

    if (p->pos.y > WORLD_MARGIN)
        p->flag |= FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY;
    else if (p->pos.y < -WORLD_MARGIN)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Y;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PY;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY);

    if (p->pos.z > WORLD_MARGIN_V)
        p->flag |= FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ;
    else if (p->pos.z < -WORLD_MARGIN_V)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Z;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PZ;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ);

    /* ---- overflow edge --------------------------------------------------- */

    if (p->pos.x > OVERFLOW_EDGE)
    {
        p->pos.x -= DIAMETER;
        p->pos_last.x -= DIAMETER;
    }
    if (p->pos.x < -OVERFLOW_EDGE)
    {
        p->pos.x += DIAMETER;
        p->pos_last.x += DIAMETER;
    }
    if (p->pos.y > OVERFLOW_EDGE)
    {
        p->pos.y -= DIAMETER;
        p->pos_last.y -= DIAMETER;
    }
    if (p->pos.y < -OVERFLOW_EDGE)
    {
        p->pos.y += DIAMETER;
        p->pos_last.y += DIAMETER;
    }
    if (p->pos.z > OVERFLOW_EDGE_V)
    {
        p->pos.z -= DIAMETER_V;
        p->pos_last.z -= DIAMETER_V;
    }
    if (p->pos.z < -OVERFLOW_EDGE_V)
    {
        p->pos.z += DIAMETER_V;
        p->pos_last.z += DIAMETER_V;
    }
}

void player_camera_movement_update(Player *p, v2f64 mouse_delta, b8 use_mouse)
{
    f32 zoom, sensitivity, spch, cpch, syaw, cyaw;

    if (use_mouse)
    {
        if (p->flag & FLAG_PLAYER_ZOOMER)
            zoom = p->camera.zoom;
        else zoom = 0.0f;

        sensitivity = settings.mouse_sensitivity /
            (zoom / CAMERA_ZOOM_SENSITIVITY + 1.0f);

        p->yaw += mouse_delta.x * sensitivity;
        p->pitch += mouse_delta.y * sensitivity;

        p->yaw = fmodf(p->yaw, CAMERA_RANGE_MAX);
        if (p->yaw < 0.0f)
            p->yaw += CAMERA_RANGE_MAX;

        p->pitch = clamp_f32(p->pitch, -CAMERA_ANGLE_MAX, CAMERA_ANGLE_MAX);
    }

    syaw = sin(p->yaw * DEG2RAD);
    cyaw = cos(p->yaw * DEG2RAD);
    spch = sin(p->pitch * DEG2RAD);
    cpch = cos(p->pitch * DEG2RAD);
    p->sin_yaw = syaw;
    p->cos_yaw = cyaw;
    p->sin_pitch = spch;
    p->cos_pitch = cpch;
    p->camera.sin_yaw = syaw;
    p->camera.cos_yaw = cyaw;
    p->camera.sin_pitch = spch;
    p->camera.cos_pitch = cpch;

    switch (p->camera_mode)
    {
        case MODE_CAMERA_1ST_PERSON:
            p->camera.pos =
                (v3f32){
                    p->pos.x,
                    p->pos.y,
                    p->pos.z + p->eye_height
                };
            break;

        case MODE_CAMERA_3RD_PERSON:
            p->camera.pos =
                (v3f32){
                    p->pos.x - cyaw * cpch * p->camera_distance,
                    p->pos.y + syaw * cpch * p->camera_distance,
                    p->pos.z + p->eye_height + spch * p->camera_distance,
                };
            break;

        case MODE_CAMERA_3RD_PERSON_FRONT:
            p->camera.pos =
                (v3f32){
                    p->pos.x + cyaw * cpch * p->camera_distance,
                    p->pos.y - syaw * cpch * p->camera_distance,
                    p->pos.z + p->eye_height - spch * p->camera_distance,
                };
            p->camera.sin_pitch = -spch;
            p->camera.sin_yaw = sin((p->yaw + CAMERA_RANGE_MAX / 2.0f) * DEG2RAD);
            p->camera.cos_yaw = cos((p->yaw + CAMERA_RANGE_MAX / 2.0f) * DEG2RAD);
            break;

            /* TODO: make the stalker camera mode */
        case MODE_CAMERA_STALKER:
            break;

            /* TODO: make the spectator camera mode */
        case MODE_CAMERA_SPECTATOR:
            break;
    }

    p->camera_hud.sin_pitch = p->camera.sin_pitch;
    p->camera_hud.cos_pitch = p->camera.cos_pitch;
    p->camera_hud.sin_yaw = p->camera.sin_yaw;
    p->camera_hud.cos_yaw = p->camera.cos_yaw;
}

void player_target_update(Player *p)
{
    f32 spch = p->sin_pitch;
    f32 cpch = p->cos_pitch;
    f32 syaw = p->sin_yaw;
    f32 cyaw = p->cos_yaw;

    p->target = (v3f64){
        p->pos.x + cyaw * cpch * settings.reach_distance,
        p->pos.y - syaw * cpch * settings.reach_distance,
        p->pos.z + p->eye_height - spch * settings.reach_distance,
    };

    p->target_snapped = (v3i64){
        (i64)floor(p->target.x),
        (i64)floor(p->target.y),
        (i64)floor(p->target.z),
    };
}

void set_player_pos(Player *p, f64 x, f64 y, f64 z)
{
    p->pos = (v3f64){x, y, z};
    p->pos_last = p->pos;
}

void set_player_block(Player *p, i64 x, i64 y, i64 z)
{
    p->pos = (v3f64){(f64)x + 0.5f, (f64)y + 0.5f, (f64)z + 0.5f};
    p->pos_last = p->pos;
}

void set_player_spawn(Player *p, i64 x, i64 y, i64 z)
{
    p->spawn = (v3i64){x, y, z};
}

void player_kill(Player *p)
{
    p->movement = (v3f32){0};
    p->velocity = (v3f32){0};
    p->gravity_influence = (v3f32){0};
    p->flag |= FLAG_PLAYER_DEAD;
}

void player_spawn(Player *p, b8 hard)
{
    set_player_pos(p,
            p->spawn.x + 0.5f,
            p->spawn.y + 0.5f,
            p->spawn.z + 0.5f);
    if (!hard) return;
    p->flag &= ~(FLAG_PLAYER_FLYING | FLAG_PLAYER_HUNGRY | FLAG_PLAYER_DEAD);
}

#if 1
void player_collision_update(Player *p, f64 dt)
{
    if (!MODE_INTERNAL_COLLIDE) return;

    BoundingBox r = {0};
    v3f32 diff = {0};
    v3f32 normal = {0};
    f32 collision_time = 0;

    v3f64 box_1[2] =
    {
        {
            p->pos.x - p->size.x * 0.5f,
            p->pos.y - p->size.y * 0.5f,
            p->pos.z,
        },
        {
            p->pos.x + p->size.x * 0.5f,
            p->pos.y + p->size.y * 0.5f,
            p->pos.z + p->size.z,
        },
    };

    v3f64 box_2[2] = {0};
    Chunk *ch = NULL;
    u32 *block = NULL;
    u32 chunk_index = 0;
    i32 x = 0, y = 0, z = 0;

    r = make_aabb_broad_phase_region(p, SET_BP_REGION_MARGIN, dt);
    p->bp_region = r;
    r.pos = (v3i64){
        r.pos.x - p->chunk.x * CHUNK_DIAMETER,
        r.pos.y - p->chunk.y * CHUNK_DIAMETER,
        r.pos.z - p->chunk.z * CHUNK_DIAMETER,
    };

    for (z = r.pos.z; z < r.pos.z + r.size.z; ++z)
        for (y = r.pos.y; y < r.pos.y + r.size.y; ++y)
            for (x = r.pos.x; x < r.pos.x + r.size.x; ++x)
            {
                ch = get_chunk_resolved(settings.chunk_tab_center, x, y, z);
                if (!ch) continue;
                block = get_block_resolved(ch, x, y, z);
                if (!block || !*block) continue;

                box_2[0] = (v3f64){
                    (f64)((i64)ch->pos.x * CHUNK_DIAMETER + mod(x, CHUNK_DIAMETER)),
                    (f64)((i64)ch->pos.y * CHUNK_DIAMETER + mod(y, CHUNK_DIAMETER)),
                    (f64)((i64)ch->pos.z * CHUNK_DIAMETER + mod(z, CHUNK_DIAMETER)),
                };
                box_2[1] = (v3f64){
                    box_2[0].x + 1.0f,
                    box_2[0].y + 1.0f,
                    box_2[0].z + 1.0f,
                };

                collision_time = get_aabb_collision_time(box_1, box_2,
                        p->velocity, &diff, &normal);

                if (is_in_range_f32(collision_time, 0.0f, 1.0f))
                {

                    if (is_in_range_f32(diff.x, 0.0f, 1.0f))
                    {
                        p->gravity_influence.x = 0.0f;
                        p->pos.x += p->velocity.x * dt * diff.x;
                    }

                    if (is_in_range_f32(diff.y, 0.0f, 1.0f))
                    {
                        p->gravity_influence.y = 0.0f;
                        p->pos.y += p->velocity.y * dt * diff.y;
                    }

                    if (is_in_range_f32(diff.z, 0.0f, 1.0f))
                    {
                        p->gravity_influence.z = 0.0f;
                        p->flag |= FLAG_PLAYER_CAN_JUMP;
                        p->flag &= ~FLAG_PLAYER_FLYING;
                        p->flag &= ~FLAG_PLAYER_MID_AIR;
                        p->pos.z -= p->velocity.z * dt * diff.z;
                    }

                    box_1[0] = (v3f64){
                        p->pos.x - p->size.x * 0.5f,
                        p->pos.y - p->size.y * 0.5f,
                        p->pos.z,
                    };

                    box_1[1] = (v3f64){
                        p->pos.x + p->size.x * 0.5f,
                        p->pos.y + p->size.y * 0.5f,
                        p->pos.z + p->size.z,
                    };
                }
            }
}
#else
void player_collision_update(Player *p, f64 dt)
{
    if (!MODE_INTERNAL_COLLIDE) return;

    BoundingBox r = {0};
    v3f32 diff = {0};
    v3f32 normal = {0};
    f32 collision_time = 0;

    v3f64 box_1[2] =
    {
        {
            p->pos.x - p->size.x * 0.5f,
            p->pos.y - p->size.y * 0.5f,
            p->pos.z,
        },
        {
            p->pos.x + p->size.x * 0.5f,
            p->pos.y + p->size.y * 0.5f,
            p->pos.z + p->size.z,
        },
    };

    v3f64 box_2[2] = {0};
    Chunk *ch = NULL;
    u32 *block = NULL;
    u32 chunk_index = 0;
    i32 x = 0, y = 0, z = 0,
        dx = 0, dy = 0, dz = 0,
        dchx = 0, dchy = 0, dchz = 0;

    r = make_aabb_broad_phase_region(p, SET_BP_REGION_MARGIN, dt);
    p->bp_region = r;
    r.pos = (v3i64){
        r.pos.x - p->chunk.x * CHUNK_DIAMETER,
        r.pos.y - p->chunk.y * CHUNK_DIAMETER,
        r.pos.z - p->chunk.z * CHUNK_DIAMETER,
    };

    for (z = r.pos.z; z < r.pos.z + r.size.z; ++z)
    {
        dz = mod(z, CHUNK_DIAMETER);
        dchz = (i32)floorf((f32)z / CHUNK_DIAMETER);
        for (y = r.pos.y; y < r.pos.y + r.size.y; ++y)
        {
            dy = mod(y, CHUNK_DIAMETER);
            dchy = (i32)floorf((f32)y / CHUNK_DIAMETER);
            for (x = r.pos.x; x < r.pos.x + r.size.x; ++x)
            {
                dx = mod(x, CHUNK_DIAMETER);
                dchx = (i32)floorf((f32)x / CHUNK_DIAMETER);

                ch = chunk_tab[settings.chunk_tab_center + dchx +
                    dchy * settings.chunk_buf_diameter +
                    dchz * settings.chunk_buf_layer];
                if (!ch) continue;
                block = &ch->block[dz][dy][dx];
                if (!block || !*block) continue;

                box_2[0] = (v3f64){
                    (f64)((i64)ch->pos.x * CHUNK_DIAMETER + dx),
                    (f64)((i64)ch->pos.y * CHUNK_DIAMETER + dy),
                    (f64)((i64)ch->pos.z * CHUNK_DIAMETER + dz),
                };
                box_2[1] = (v3f64){
                    box_2[0].x + 1.0f,
                    box_2[0].y + 1.0f,
                    box_2[0].z + 1.0f,
                };

                return;
                collision_time = get_aabb_collision_time(box_1, box_2,
                        p->velocity, &diff, &normal);

                if (is_in_range_f32(collision_time, 0.0f, 1.0f))
                {

                    if (is_in_range_f32(diff.x, 0.0f, 1.0f))
                    {
                        p->gravity_influence.x = 0.0f;
                        p->pos.x += p->velocity.x * dt * diff.x;
                    }

                    if (is_in_range_f32(diff.y, 0.0f, 1.0f))
                    {
                        p->gravity_influence.y = 0.0f;
                        p->pos.y += p->velocity.y * dt * diff.y;
                    }

                    if (is_in_range_f32(diff.z, 0.0f, 1.0f))
                    {
                        p->gravity_influence.z = 0.0f;
                        p->flag |= FLAG_PLAYER_CAN_JUMP;
                        p->flag &= ~FLAG_PLAYER_FLYING;
                        p->flag &= ~FLAG_PLAYER_MID_AIR;
                        p->pos.z -= p->velocity.z * dt * diff.z;
                    }

                    box_1[0] = (v3f64){
                        p->pos.x - p->size.x * 0.5f,
                        p->pos.y - p->size.y * 0.5f,
                        p->pos.z,
                    };

                    box_1[1] = (v3f64){
                        p->pos.x + p->size.x * 0.5f,
                        p->pos.y + p->size.y * 0.5f,
                        p->pos.z + p->size.z,
                    };
                }
            }
        }
    }
}
#endif

BoundingBox make_aabb_broad_phase_region(Player *p, f32 padding, f64 dt)
{
    v3f64 pos = {0};
    v3f32 size = {0};
    v3f32 delta = {0};
    v3f32 velocity =
    {
        p->velocity.x * dt,
        p->velocity.y * dt,
        p->velocity.z * dt,
    };

    if (padding < 1.0f)
        padding = 1.0f;

    if (velocity.x > 0.0f)
    {
        pos.x = p->pos.x - p->size.x * 0.5f - padding;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = p->size.x + velocity.x + delta.x + padding * 2.0f + 1.0f;
    }
    else
    {
        pos.x = p->pos.x + velocity.x - p->size.x * 0.5f - padding;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = p->size.x - velocity.x + delta.x + padding * 2.0f + 1.0f;
    }

    if (velocity.y > 0.0f)
    {
        pos.y = p->pos.y - p->size.y * 0.5f - padding;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = p->size.y + velocity.y + delta.y + padding * 2.0f + 1.0f;
    }
    else
    {
        pos.y = p->pos.y + velocity.y - p->size.y * 0.5f - padding;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = p->size.y - velocity.y + delta.y + padding * 2.0f + 1.0f;
    }

    if (velocity.z > 0.0f)
    {
        pos.z = p->pos.z - padding;
        delta.z = (f32)(pos.z - ceil(pos.z));
        size.z = p->size.z + velocity.z + delta.z + padding * 2.0f + 1.0f;
    }
    else
    {
        pos.z = p->pos.z + velocity.z - padding;
        delta.z = (f32)(pos.z - ceil(pos.z));
        size.z = p->size.z - velocity.z + delta.z + padding * 2.0f + 1.0f;
    }

    return (BoundingBox){
        .pos = (v3i64){
            (i64)floor(pos.x),
            (i64)floor(pos.y),
            (i64)floor(pos.z),
        },

        .size = (v3i32){
            (i32)ceilf(size.x),
            (i32)ceilf(size.y),
            (i32)ceilf(size.z),
        },
    };
}

b8 is_aabb_intersect(v3f64 a[2], v3f64 b[2])
{
    v3u8 is_intersect =
    {
        (a[0].x < b[1].x) && (a[1].x > b[0].x),
        (a[0].y < b[1].y) && (a[1].y > b[0].y),
        (a[0].z < b[1].z) && (a[1].z > b[0].z),
    };

    return is_intersect.x && is_intersect.y && is_intersect.z;
}

f32 get_aabb_collision_time(v3f64 a[2], v3f64 b[2], v3f32 velocity,
        v3f32 *diff, v3f32 *normal)
{
    v3f32 entry = {0},
          exit = {0},
          entry_inv = {0},
          exit_inv = {0};
    f32 entry_time = 0,
        exit_time = 0;

    *normal = (v3f32){0};

    /* ---- inverse entry and exit ------------------------------------------ */

    if (velocity.x > 0.0f)
    {
        entry_inv.x = b[0].x - a[1].x;
        exit_inv.x = b[1].x - a[0].x;
    }
    else
    {
        entry_inv.x = b[1].x - a[0].x;
        exit_inv.x = b[0].x - a[1].x;
    }

    if (velocity.y > 0.0f)
    {
        entry_inv.y = b[0].y - a[1].y;
        exit_inv.y = b[1].y - a[0].y;
    }
    else
    {
        entry_inv.y = b[1].y - a[0].y;
        exit_inv.y = b[0].y - a[1].y;
    }
    if (velocity.z > 0.0f)
    {
        entry_inv.z = b[0].z - a[1].z;
        exit_inv.z = b[1].z - a[0].z;
    }
    else
    {
        entry_inv.z = b[1].z - a[0].z;
        exit_inv.z = b[0].z - a[1].z;
    }

    /* ---- entry and exit -------------------------------------------------- */

    entry = (v3f32){
        entry_inv.x / velocity.x,
        entry_inv.y / velocity.y,
        entry_inv.z / velocity.z,
    };

    exit = (v3f32){
        exit_inv.x / velocity.x,
        exit_inv.y / velocity.y,
        exit_inv.z / velocity.z,
    };

    entry_time = max_v3f32(entry);
    exit_time = min_v3f32(exit);
    printf("time[%f][%f] diff[%f %f %f]\n",
            entry_time, exit_time, entry_inv.x, entry_inv.y, entry_inv.z);

    if (entry_time > exit_time ||
            (entry.x < 0.0f && entry.y < 0.0f && entry.z < 0.0f) ||
            entry.x > 1.0f || entry.y > 1.0f || entry.z > 1.0f)
        return -1.0f;

    /* ---- normals --------------------------------------------------------- */

    switch (max_axis_v3f32(entry))
    {
        case 1:
            if (entry_inv.x < 0.0f)
                normal->x = 1.0f;
            else
                normal->x = -1.0f;
            break;

        case 2:
            if (entry_inv.y < 0.0f)
                normal->y = 1.0f;
            else
                normal->y = -1.0f;
            break;

        case 3:
            if (entry_inv.z < 0.0f)
                normal->z = 1.0f;
            else
                normal->z = -1.0f;
            break;
    }

    *diff = entry;
    return entry_time;
}

void gravity_update(v3f64 *movement, v3f32 *influence, f32 weight, f64 dt)
{
    influence->z += GRAVITY * weight * dt;
    movement->x += influence->x * dt;
    movement->y += influence->y * dt;
    movement->z += influence->z * dt;
}
