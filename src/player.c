#include <engine/h/collision.h>
#include <engine/h/math.h>

#include "h/chunking.h"
#include "h/player.h"
#include "h/world.h"

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief handle player being near or past world edges.
 *
 *  teleport player to the other side of the world if they cross a world edge.
 */
static void player_wrap_coordinates(Player *p);

void player_update(Player *p, f64 dt)
{
    v3f32 control = {0}, drag = {0}, gravity = {0};

    p->acceleration = (v3f32){0};
    p->acceleration_rate = PLAYER_ACCELERATION_WALK;
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

        p->acceleration_rate = PLAYER_ACCELERATION_FLY;

        if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        {
            p->drag.x = SET_DRAG_FLY_NATURAL;
            p->drag.y = SET_DRAG_FLY_NATURAL;
            p->drag.z = SET_DRAG_FLY_NATURAL;
            p->friction = (v3f32){0};
        }
        else
        {
            p->drag.x = SET_DRAG_FLYING;
            p->drag.y = SET_DRAG_FLYING;
            p->drag.z = SET_DRAG_FLYING_V;
        }

        p->camera.fovy += 10.0f;

        if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->acceleration_rate = PLAYER_ACCELERATION_FLY_FAST;
            p->camera.fovy += 10.0f;
        }
    }
    else
    {
        gravity.z -= world.gravity;
        p->drag.x = map_range_f32(p->friction.x, 0.0f, 1.0f, SET_DRAG_AIR, SET_DRAG_GROUND_SOLID);
        p->drag.y = map_range_f32(p->friction.y, 0.0f, 1.0f, SET_DRAG_AIR, SET_DRAG_GROUND_SOLID);
        p->drag.z = map_range_f32(p->friction.z, 0.0f, 1.0f, SET_DRAG_AIR, SET_DRAG_GROUND_SOLID);

        if (!(p->flag &FLAG_PLAYER_CAN_JUMP))
            p->flag |= FLAG_PLAYER_MID_AIR;

        if (p->flag & FLAG_PLAYER_SNEAKING)
            p->acceleration_rate = PLAYER_ACCELERATION_SNEAK;
        else if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->acceleration_rate = PLAYER_ACCELERATION_SPRINT;
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

    p->acceleration = (v3f32){
        control.x - drag.x + gravity.x,
        control.y - drag.y + gravity.y,
        control.z - drag.z + gravity.z,
    };

    p->velocity.x += p->acceleration.x * dt;
    p->velocity.y += p->acceleration.y * dt;
    p->velocity.z += p->acceleration.z * dt;

    p->pos.x += p->velocity.x * dt;
    p->pos.y += p->velocity.y * dt;
    p->pos.z += p->velocity.z * dt;

    p->speed = sqrtf(len_v3f32(p->velocity));
    if (p->speed > EPSILON)
        p->camera.fovy += p->speed * 0.03f;
    p->camera.fovy = clamp_f32(p->camera.fovy, 1.0f, SET_FOV_MAX);
    p->camera.fovy_smooth = lerp_f32(p->camera.fovy_smooth, p->camera.fovy,
                dt, SET_LERP_SPEED_FOV_MODE);

    player_bounding_box_update(p);
    player_collision_update(p, dt);
    player_wrap_coordinates(p);
    player_chunk_update(p);
}

void player_collision_update(Player *p, f64 dt)
{
    if (!MODE_INTERNAL_COLLIDE) return;

    Chunk *ch = NULL;
    u32 *block = NULL;
    v3f32 displacement =
    {
        p->velocity.x * dt,
        p->velocity.y * dt,
        p->velocity.z * dt,
    };
    f32 time = 0.0f;
    v3f32 normal = {0};
    f32 dot = 0.0f;
    BoundingBox block_box = {0};
    BoundingBox collision_capsule;
    i32 i, x, y, z,
        startx, starty, startz,
        endx, endy, endz;
    b8 resolved = TRUE;

    collision_capsule = make_collision_capsule(p->bbox, p->chunk, displacement);
    startx = (i32)collision_capsule.pos.x;
    starty = (i32)collision_capsule.pos.y;
    startz = (i32)collision_capsule.pos.z;
    endx = (i32)(collision_capsule.pos.x + collision_capsule.size.x);
    endy = (i32)(collision_capsule.pos.y + collision_capsule.size.y);
    endz = (i32)(collision_capsule.pos.z + collision_capsule.size.z);

    p->flag &= ~FLAG_PLAYER_CAN_JUMP;
    p->friction.x = 0.0f;
    p->friction.y = 0.0f;

    for (i = 0; i < 3 && resolved; ++i)
    {
        resolved = FALSE;
        for (z = startz; z < endz; ++z)
        {
            for (y = starty; y < endy; ++y)
            {
                for (x = startx; x < endx; ++x)
                {
                    ch = get_chunk_resolved(settings.chunk_tab_center, x, y, z);
                    if (!ch || !(ch->flag & FLAG_CHUNK_GENERATED)) continue;
                    block = get_block_resolved(ch, x, y, z);
                    if (!block || !*block) continue;

                    block_box.pos.x = (f64)((i64)ch->pos.x * CHUNK_DIAMETER + mod(x, CHUNK_DIAMETER));
                    block_box.pos.y = (f64)((i64)ch->pos.y * CHUNK_DIAMETER + mod(y, CHUNK_DIAMETER));
                    block_box.pos.z = (f64)((i64)ch->pos.z * CHUNK_DIAMETER + mod(z, CHUNK_DIAMETER));
                    block_box.size.x = 1.0;
                    block_box.size.y = 1.0;
                    block_box.size.z = 1.0;

                    time = get_swept_aabb(p->bbox, block_box, p->velocity, &normal);
                    if (normal.z > 0.0f && p->bbox.pos.z - (block_box.pos.z + block_box.size.z) < 0.3)
                    {
                        p->friction.x = 0.7f;
                        p->friction.y = 0.7f;
                    }
                    if (is_intersect_aabb(p->bbox, block_box))
                    {
                        /* ---- resolution ---------------------------------- */

                        printf("x[%u] i[%u] v[%.3f %.3f %.3f] n[%.0f %.0f %.0f]\n",
                                x, i, p->velocity.x, p->velocity.y, p->velocity.z,
                                normal.x, normal.y, normal.z);

                        p->pos.x += p->velocity.x * time + normal.x * COLLISION_EPSILON;
                        p->pos.y += p->velocity.y * time + normal.y * COLLISION_EPSILON;
                        p->pos.z += p->velocity.z * time + normal.z * COLLISION_EPSILON;

                        dot = dot_v3f32(p->velocity, normal);
                        if (dot < 0.0f)
                        {
                            p->velocity.x -= normal.x * dot;
                            p->velocity.y -= normal.y * dot;
                            p->velocity.z -= normal.z * dot;
                        }

                        p->pos.x -= p->velocity.x * time;
                        p->pos.y -= p->velocity.y * time;
                        p->pos.z -= p->velocity.z * time;

                        player_bounding_box_update(p);

                        /* ---- flags --------------------------------------- */

                        if (normal.z > 0.0f)
                        {
                            if (!(p->flag & FLAG_PLAYER_CINEMATIC_MOTION))
                                p->flag &= ~FLAG_PLAYER_FLYING;
                            p->flag |= FLAG_PLAYER_CAN_JUMP;
                            p->friction.x = 0.7f;
                            p->friction.y = 0.7f;
                        }

                        resolved = TRUE;
                    }
                }
            }
        }
    }
}

void player_bounding_box_update(Player *p)
{
    if (p->flag & FLAG_PLAYER_FLYING && p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
    {
        p->bbox.pos.x = p->pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = p->pos.y - p->size.x * 0.5f;
        p->bbox.pos.z = p->pos.z + p->eye_height - p->size.x * 0.5f;

        /* size for all axes is 'x' intentionally because we want cube bbox */
        p->bbox.size.x = p->size.x;
        p->bbox.size.y = p->size.x;
        p->bbox.size.z = p->size.x;
    }
    else
    {
        p->bbox.pos.x = p->pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = p->pos.y - p->size.y * 0.5f;
        p->bbox.pos.z = p->pos.z;
        p->bbox.size.x = p->size.x;
        p->bbox.size.y = p->size.y;
        p->bbox.size.z = p->size.z;
    }
}

BoundingBox make_collision_capsule(BoundingBox b, v3i32 chunk, v3f32 velocity)
{
    const f32 PADDING = 1.0f;
    v3f64 pos = {0};
    v3f64 size = {0};
    v3f32 delta = {0};

    if (velocity.x > 0.0f)
    {
        pos.x = b.pos.x - b.size.x * 0.5f - PADDING;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = b.size.x + velocity.x + delta.x + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.x = b.pos.x + velocity.x - b.size.x * 0.5f - PADDING;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = b.size.x - velocity.x + delta.x + PADDING * 2.0f + 1.0f;
    }

    if (velocity.y > 0.0f)
    {
        pos.y = b.pos.y - b.size.y * 0.5f - PADDING;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = b.size.y + velocity.y + delta.y + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.y = b.pos.y + velocity.y - b.size.y * 0.5f - PADDING;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = b.size.y - velocity.y + delta.y + PADDING * 2.0f + 1.0f;
    }

    if (velocity.z > 0.0f)
    {
        pos.z = b.pos.z - PADDING;
        delta.z = (f32)(pos.z - ceil(pos.z));
        size.z = b.size.z + velocity.z + delta.z + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.z = b.pos.z + velocity.z - PADDING;
        delta.z = (f32)(pos.z - ceil(pos.z));
        size.z = b.size.z - velocity.z + delta.z + PADDING * 2.0f + 1.0f;
    }

    return (BoundingBox){
        .pos = (v3f64){
            floor(pos.x) - chunk.x * CHUNK_DIAMETER,
            floor(pos.y) - chunk.y * CHUNK_DIAMETER,
            floor(pos.z) - chunk.z * CHUNK_DIAMETER,
        },

        .size = (v3f64){
            ceil(size.x),
            ceil(size.y),
            ceil(size.z),
        },
    };
}

void player_chunk_update(Player *p)
{
    p->chunk.x = floorf((f32)p->pos.x / CHUNK_DIAMETER);
    p->chunk.y = floorf((f32)p->pos.y / CHUNK_DIAMETER);
    p->chunk.z = floorf((f32)p->pos.z / CHUNK_DIAMETER);

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
    f32 zoom, sensitivity, srol, crol, spch, cpch, syaw, cyaw;

    if (use_mouse)
    {
        if (p->flag & FLAG_PLAYER_ZOOMER)
            zoom = p->camera.zoom;
        else zoom = 0.0f;

        sensitivity = settings.mouse_sensitivity / (zoom / CAMERA_ZOOM_SENSITIVITY + 1.0f);

        p->pitch += mouse_delta.y * sensitivity;
        p->yaw += mouse_delta.x * sensitivity;

        p->pitch = clamp_f32(p->pitch, -CAMERA_ANGLE_MAX, CAMERA_ANGLE_MAX);
        p->yaw = fmodf(p->yaw, CAMERA_RANGE_MAX);
        if (p->yaw < 0.0f)
            p->yaw += CAMERA_RANGE_MAX;
    }

    srol = sin(p->roll * DEG2RAD);
    crol = cos(p->roll * DEG2RAD);
    spch = sin(p->pitch * DEG2RAD);
    cpch = cos(p->pitch * DEG2RAD);
    syaw = sin(p->yaw * DEG2RAD);
    cyaw = cos(p->yaw * DEG2RAD);
    p->sin_roll = syaw;
    p->cos_roll = cyaw;
    p->sin_pitch = spch;
    p->cos_pitch = cpch;
    p->sin_yaw = syaw;
    p->cos_yaw = cyaw;
    p->camera.sin_roll = srol;
    p->camera.cos_roll = crol;
    p->camera.sin_pitch = spch;
    p->camera.cos_pitch = cpch;
    p->camera.sin_yaw = syaw;
    p->camera.cos_yaw = cyaw;

    switch (p->camera_mode)
    {
        case PLAYER_CAMERA_MODE_1ST_PERSON:
            p->camera.pos.x = p->pos.x;
            p->camera.pos.y = p->pos.y;
            p->camera.pos.z = p->pos.z + p->eye_height;
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON:
            p->camera.pos.x = p->pos.x - cyaw * cpch * p->camera_distance;
            p->camera.pos.y = p->pos.y + syaw * cpch * p->camera_distance;
            p->camera.pos.z = p->pos.z + p->eye_height + spch * p->camera_distance;
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON_FRONT:
            p->camera.pos.x = p->pos.x + cyaw * cpch * p->camera_distance;
            p->camera.pos.y = p->pos.y - syaw * cpch * p->camera_distance;
            p->camera.pos.z = p->pos.z + p->eye_height - spch * p->camera_distance;

            p->camera.sin_pitch = -spch;
            p->camera.sin_yaw = sin((p->yaw + CAMERA_RANGE_MAX / 2.0f) * DEG2RAD);
            p->camera.cos_yaw = cos((p->yaw + CAMERA_RANGE_MAX / 2.0f) * DEG2RAD);
            break;

            /* TODO: make the stalker camera mode */
        case PLAYER_CAMERA_MODE_STALKER:
            break;

            /* TODO: make the spectator camera mode */
        case PLAYER_CAMERA_MODE_SPECTATOR:
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

    p->target.x = p->pos.x + cyaw * cpch * settings.reach_distance;
    p->target.y = p->pos.y - syaw * cpch * settings.reach_distance;
    p->target.z = p->pos.z + p->eye_height - spch * settings.reach_distance;

    p->target_snapped.x = (i64)floor(p->target.x);
    p->target_snapped.y = (i64)floor(p->target.y);
    p->target_snapped.z = (i64)floor(p->target.z);
}

void set_player_pos(Player *p, f64 x, f64 y, f64 z)
{
    p->pos = (v3f64){x, y, z};
    p->velocity = (v3f32){0};
    p->pos_last = p->pos;
}

void set_player_block(Player *p, i64 x, i64 y, i64 z)
{
    p->pos.x = (f64)x + 0.5f;
    p->pos.y = (f64)y + 0.5f;
    p->pos.z = (f64)z + 0.5f;
    p->pos_last = p->pos;
}

void set_player_spawn(Player *p, i64 x, i64 y, i64 z)
{
    p->spawn = (v3i64){x, y, z};
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

void player_kill(Player *p)
{
    p->acceleration = (v3f32){0};
    p->velocity = (v3f32){0};
    p->flag |= FLAG_PLAYER_DEAD;
}
