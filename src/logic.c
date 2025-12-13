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
    v3f32 control = {0}, drag = {0}, forces = {0};
    f32 gravity = 0.0f;

    p->acceleration = (v3f32){0};
    p->acceleration_rate = SET_PLAYER_ACCELERATION_WALK;
    p->drag = (v3f32){
        SET_DRAG_AIR,
        SET_DRAG_AIR,
        SET_DRAG_AIR,
    };
    p->camera.fovy = settings.fov;

    /* ---- player flags ---------------------------------------------------- */

    if (p->flag & FLAG_PLAYER_FLYING)
    {
        p->flag &= ~(FLAG_PLAYER_CAN_JUMP | FLAG_PLAYER_MID_AIR);

        p->acceleration_rate = SET_PLAYER_ACCELERATION_FLY;

        if (p->fly_natural)
            p->drag = (v3f32){
                SET_DRAG_AIR,
                SET_DRAG_AIR,
                SET_DRAG_AIR,
            };
        else
            p->drag = (v3f32){
                SET_DRAG_FLYING,
                SET_DRAG_FLYING,
                SET_DRAG_FLYING_V,
            };

        p->camera.fovy += 10.0f;

        if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->acceleration_rate = SET_PLAYER_ACCELERATION_FLY_FAST;
            p->camera.fovy += 10.0f;
        }
    }
    else
    {
        gravity = -GRAVITY;
        p->drag = (v3f32){
            SET_DRAG_AIR,
            SET_DRAG_AIR,
            SET_DRAG_AIR,
        };

        if (!(p->flag &FLAG_PLAYER_CAN_JUMP))
            p->flag |= FLAG_PLAYER_MID_AIR;

        if (p->flag & FLAG_PLAYER_SNEAKING)
            p->acceleration_rate = SET_PLAYER_ACCELERATION_SNEAK;
        else if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->acceleration_rate = SET_PLAYER_ACCELERATION_SPRINT;
            p->camera.fovy += 5.0f;
        }
    }

    if (p->flag & FLAG_PLAYER_ZOOMER && p->camera.zoom)
        p->camera.fovy = settings.fov - p->camera.zoom;

    /* ---- apply parameters ------------------------------------------------ */

    control = (v3f32){
        p->input.x * p->acceleration_rate * p->drag.x,
        p->input.y * p->acceleration_rate * p->drag.y,
        p->input.z * p->acceleration_rate * p->drag.z,
    };

    drag = (v3f32){
        p->drag.x * p->velocity.x,
        p->drag.y * p->velocity.y,
        p->drag.z * p->velocity.z,
    };

    forces = (v3f32){
        0.0f,
        0.0f,
        gravity,
    };

    p->acceleration = (v3f32){
        control.x - drag.x + forces.x,
        control.y - drag.y + forces.y,
        control.z - drag.z + forces.z,
    };

    p->velocity.x += p->acceleration.x * dt;
    p->velocity.y += p->acceleration.y * dt;
    p->velocity.z += p->acceleration.z * dt;

    p->speed = sqrtf(len_v3f32(p->velocity));
    if (p->speed > EPSILON)
        p->camera.fovy += p->speed * 0.03f;
    p->camera.fovy = clamp_f32(p->camera.fovy, 1.0f, SET_FOV_MAX);
    p->camera.fovy_smooth = lerp_f32(p->camera.fovy_smooth, p->camera.fovy,
                dt, SET_LERP_SPEED_FOV_MODE);


    if (!player_collision_update(p, dt))
    {
        p->pos = (v3f64){
            p->pos.x + p->velocity.x * dt,
            p->pos.y + p->velocity.y * dt,
            p->pos.z + p->velocity.z * dt,
        };

        p->bbox.pos.x = p->pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = p->pos.y - p->size.y * 0.5f;
        p->bbox.pos.z = p->pos.z;
        p->bbox.size = p->size;
    }

    player_wrap_coordinates(p);
    player_chunk_update(p);
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
                -(f64)(WORLD_DIAMETER + WORLD_RADIUS) * CHUNK_DIAMETER,
                -(f64)(WORLD_DIAMETER + WORLD_RADIUS) * CHUNK_DIAMETER,
                -(f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER)},
                (v3f64){
                (f64)(WORLD_DIAMETER + WORLD_RADIUS) * CHUNK_DIAMETER,
                (f64)(WORLD_DIAMETER + WORLD_RADIUS) * CHUNK_DIAMETER,
                (f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL) * CHUNK_DIAMETER}))
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
    p->velocity = (v3f32){0};
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
    p->acceleration = (v3f32){0};
    p->velocity = (v3f32){0};
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

b8 player_collision_update(Player *p, f64 dt)
{
    if (!MODE_INTERNAL_COLLIDE) return FALSE;

    u32 resolved = 0;
    f32 time = 0.0f;
    f32 best_time = 1.0f;
    f32 remaining_time = 1.0f;
    v3f32 velocity = p->velocity;
    v3f64 pos = p->pos;
    v3f32 normal = {0};
    v3f32 best_normal = {0};
    f32 dot = 0.0f;
    Chunk *ch = NULL;
    Chunk *best_ch = NULL;
    u32 *block = NULL;
    u32 *best_block = NULL;
    BoundingBox block_box = {0};
    BoundingBox best_block_box = {0};
    i32 i, x, y, z;

    for (i = 0; i < SET_COLLISION_SWEEP_MAX; ++i)
    {
        p->bbox.pos.x = pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = pos.y - p->size.y * 0.5f;
        p->bbox.pos.z = pos.z;
        p->bbox.size = p->size;

        p->capsule = make_collision_capsule(p->bbox, p->chunk, velocity,
                SET_COLLISION_CAPSULE_PADDING, dt);

        for (z = p->capsule.pos.z; z < p->capsule.pos.z + p->capsule.size.z; ++z)
            for (y = p->capsule.pos.y; y < p->capsule.pos.y + p->capsule.size.y; ++y)
                for (x = p->capsule.pos.x; x < p->capsule.pos.x + p->capsule.size.x; ++x)
                {
                    ch = get_chunk_resolved(settings.chunk_tab_center, x, y, z);
                    if (!ch || !(ch->flag & FLAG_CHUNK_GENERATED)) continue;
                    block = get_block_resolved(ch, x, y, z);
                    if (!block || !*block) continue;

                    block_box.pos.x = (f64)((i64)ch->pos.x * CHUNK_DIAMETER + mod(x, CHUNK_DIAMETER));
                    block_box.pos.y = (f64)((i64)ch->pos.y * CHUNK_DIAMETER + mod(y, CHUNK_DIAMETER));
                    block_box.pos.z = (f64)((i64)ch->pos.z * CHUNK_DIAMETER + mod(z, CHUNK_DIAMETER));
                    block_box.size = (v3f32){1.0f, 1.0f, 1.0f};

                    time = get_swept_aabb(p->bbox, block_box, velocity, &normal);

                    if (time < best_time)
                    {
                        best_ch = ch;
                        best_time = time;
                        best_normal = normal;
                        best_block = block;
                        best_block_box = block_box;
                    }
                }

        if (!is_intersect_aabb(p->bbox, best_block_box))
        {
            p->pos.x += p->velocity.x * dt;
            p->pos.y += p->velocity.y * dt;
            p->pos.z += p->velocity.z * dt;
            return FALSE;
        }

        pos.x += velocity.x * dt * best_time;
        pos.y += velocity.y * dt * best_time;
        pos.z += velocity.z * dt * best_time;

        dot = dot_v3f32(velocity, best_normal);
        if (dot < 0.0f)
        {
            velocity.x -= best_normal.x * dot;
            velocity.y -= best_normal.y * dot;
            velocity.z -= best_normal.z * dot;
        }

        remaining_time -= best_time;
        pos.x += velocity.x * dt * remaining_time;
        pos.y += velocity.y * dt * remaining_time;
        pos.z += velocity.z * dt * remaining_time;

        velocity.x *= remaining_time;
        velocity.y *= remaining_time;
        velocity.z *= remaining_time;

        if (len_v3f32(velocity) < SET_COLLISION_EPSILON * SET_COLLISION_EPSILON)
            return FALSE;

        if (best_normal.z > 0.0f)
        {
            p->flag |= FLAG_PLAYER_CAN_JUMP;
            p->flag &= ~(FLAG_PLAYER_FLYING | FLAG_PLAYER_MID_AIR);
        }
    }

    p->pos = pos;
    p->velocity = velocity;
    
    return FALSE;
}

CollisionCapsule make_collision_capsule(BoundingBox b, v3i32 chunk, v3f32 velocity, f32 padding, f64 dt)
{
    v3f64 pos = {0};
    v3f32 size = {0};
    v3f32 delta = {0};
    velocity = (v3f32){
        velocity.x * dt,
        velocity.y * dt,
        velocity.z * dt,
    };

    if (padding < 1.0f)
        padding = 1.0f;

    if (velocity.x > 0.0f)
    {
        pos.x = b.pos.x - b.size.x * 0.5f - padding;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = b.size.x + velocity.x + delta.x + padding * 2.0f + 1.0f;
    }
    else
    {
        pos.x = b.pos.x + velocity.x - b.size.x * 0.5f - padding;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = b.size.x - velocity.x + delta.x + padding * 2.0f + 1.0f;
    }

    if (velocity.y > 0.0f)
    {
        pos.y = b.pos.y - b.size.y * 0.5f - padding;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = b.size.y + velocity.y + delta.y + padding * 2.0f + 1.0f;
    }
    else
    {
        pos.y = b.pos.y + velocity.y - b.size.y * 0.5f - padding;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = b.size.y - velocity.y + delta.y + padding * 2.0f + 1.0f;
    }

    if (velocity.z > 0.0f)
    {
        pos.z = b.pos.z - padding;
        delta.z = (f32)(pos.z - ceil(pos.z));
        size.z = b.size.z + velocity.z + delta.z + padding * 2.0f + 1.0f;
    }
    else
    {
        pos.z = b.pos.z + velocity.z - padding;
        delta.z = (f32)(pos.z - ceil(pos.z));
        size.z = b.size.z - velocity.z + delta.z + padding * 2.0f + 1.0f;
    }

    return (CollisionCapsule){
        .pos = (v3i64){
            (i64)floor(pos.x) - chunk.x * CHUNK_DIAMETER,
            (i64)floor(pos.y) - chunk.y * CHUNK_DIAMETER,
            (i64)floor(pos.z) - chunk.z * CHUNK_DIAMETER,
        },

        .size = (v3i32){
            (i32)ceilf(size.x),
            (i32)ceilf(size.y),
            (i32)ceilf(size.z),
        },
    };
}

BoundingBox make_broad_phase_region(v3f64 pos, v3f32 size, v3f32 velocity, f64 dt)
{
    velocity = (v3f32){
        velocity.x * dt,
        velocity.y * dt,
        velocity.z * dt,
    };

    if (velocity.x > 0.0f)
    {
        pos.x = pos.x - size.x * 0.5f;
        size.x = size.x + velocity.x;
    }
    else
    {
        pos.x = pos.x + velocity.x - size.x * 0.5f;
        size.x = size.x - velocity.x;
    }

    if (velocity.y > 0.0f)
    {
        pos.y = pos.y - size.y * 0.5f;
        size.y = size.y + velocity.y;
    }
    else
    {
        pos.y = pos.y + velocity.y - size.y * 0.5f;
        size.y = size.y - velocity.y;
    }

    if (velocity.z > 0.0f)
    {
        pos.z = pos.z;
        size.z = size.z + velocity.z;
    }
    else
    {
        pos.z = pos.z + velocity.z;
        size.z = size.z - velocity.z;
    }

    return (BoundingBox){
        .pos = pos,
        .size = size,
    };
}

b8 is_intersect_aabb(BoundingBox a, BoundingBox b)
{
    return !(a.pos.x > b.pos.x + b.size.x || a.pos.x + a.size.x < b.pos.x ||
            a.pos.y > b.pos.y + b.size.y || a.pos.y + a.size.y < b.pos.y ||
            a.pos.z > b.pos.z + b.size.z || a.pos.z + a.size.z < b.pos.z);
}

f32 get_swept_aabb(BoundingBox a, BoundingBox b, v3f32 velocity, v3f32 *normal)
{
    v3f32 entry = {0};
    v3f32 exit = {0};
    v3f32 entry_distance = {0};
    v3f32 exit_distance = {0};
    f32 entry_time = 0.0f;
    f32 exit_time = 0.0f;

    *normal = (v3f32){0};

    /* ---- entry and exit distance ----------------------------------------- */

    if (velocity.x > 0.0f)
    {
        entry_distance.x = b.pos.x - (a.pos.x + a.size.x);
        exit_distance.x = (b.pos.x + b.size.x) - a.pos.x;
    }
    else
    {
        entry_distance.x = (b.pos.x + b.size.x) - a.pos.x;
        exit_distance.x = b.pos.x - (a.pos.x + a.size.x);
    }

    if (velocity.y > 0.0f)
    {
        entry_distance.y = b.pos.y - (a.pos.y + a.size.y);
        exit_distance.y = (b.pos.y + b.size.y) - a.pos.y;
    }
    else
    {
        entry_distance.y = (b.pos.y + b.size.y) - a.pos.y;
        exit_distance.y = b.pos.y - (a.pos.y + a.size.y);
    }

    if (velocity.z > 0.0f)
    {
        entry_distance.z = b.pos.z - (a.pos.z + a.size.z);
        exit_distance.z = (b.pos.z + b.size.z) - a.pos.z;
    }
    else
    {
        entry_distance.z = (b.pos.z + b.size.z) - a.pos.z;
        exit_distance.z = b.pos.z - (a.pos.z + a.size.z);
    }

    /* ---- entry and exit -------------------------------------------------- */

    if (velocity.x == 0.0f)
    {
        entry.x = 0.0f;
        exit.x = 0.0f;
    }
    else
    {
        entry.x = entry_distance.x / velocity.x;
        exit.x = exit_distance.x / velocity.x;
    }

    if (velocity.y == 0.0f)
    {
        entry.y = 0.0f;
        exit.y = 0.0f;
    }
    else
    {
        entry.y = entry_distance.y / velocity.y;
        exit.y = exit_distance.y / velocity.y;
    }

    if (velocity.z == 0.0f)
    {
        entry.z = 0.0f;
        exit.z = 0.0f;
    }
    else
    {
        entry.z = entry_distance.z / velocity.z;
        exit.z = exit_distance.z / velocity.z;
    }

    entry_time = max_v3f32(entry);
    exit_time = min_v3f32(exit);

    if (entry_time > exit_time || exit_time < 0.0f || entry_time > 1.0f)
        return 1.0f;

    /* ---- normals --------------------------------------------------------- */

    switch (max_axis_v3f32(entry))
    {
        case 1:
            if (velocity.x > 0.0f)
                normal->x = -1.0f;
            else
                normal->x = 1.0f;
            break;

        case 2:
            if (velocity.y > 0.0f)
                normal->y = -1.0f;
            else
                normal->y = 1.0f;
            break;

        case 3:
            if (velocity.z > 0.0f)
                normal->z = -1.0f;
            else
                normal->z = 1.0f;
            break;
    }

    return entry_time;
}
