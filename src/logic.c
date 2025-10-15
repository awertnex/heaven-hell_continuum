#include <time.h>
#include <sys/time.h>

#include <engine/h/math.h>
#include "h/main.h"
#include "h/logic.h"

void
player_state_update(Render *render, Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v)
{
    /* ---- wrap coordinates ------------------------------------------------ */
    const i64 RADIUS            = radius * chunk_diameter;
    const i64 RADIUS_V          = radius_v * chunk_diameter;
    const i64 DIAMETER          = diameter * chunk_diameter;
    const i64 DIAMETER_V        = diameter_v * chunk_diameter;

    const i64 OVERFLOW_EDGE     = (radius + 1) * chunk_diameter;
    const i64 OVERFLOW_EDGE_V   = (radius_v + 1) * chunk_diameter;

    const i64 WORLD_MARGIN =
        (radius - SET_RENDER_DISTANCE_MAX) * chunk_diameter;
    const i64 WORLD_MARGIN_V =
        (radius_v - SET_RENDER_DISTANCE_MAX) * chunk_diameter;

    /* ---- overflow edge --------------------------------------------------- */
    if (player->pos.x > OVERFLOW_EDGE)
    {
        player->pos.x -= DIAMETER;
        player->pos_smooth.x -= DIAMETER;
    }
    if (player->pos.x < -OVERFLOW_EDGE)
    {
        player->pos.x += DIAMETER;
        player->pos_smooth.x += DIAMETER;
    }

    if (player->pos.y > OVERFLOW_EDGE)
    {
        player->pos.y -= DIAMETER;
        player->pos_smooth.y -= DIAMETER;
    }
    if (player->pos.y < -OVERFLOW_EDGE)
    {
        player->pos.y += DIAMETER;
        player->pos_smooth.y += DIAMETER;
    }

    if (player->pos.z > OVERFLOW_EDGE_V)
    {
        player->pos.z -= DIAMETER_V;
        player->pos_smooth.z -= DIAMETER_V;
    }
    if (player->pos.z < -OVERFLOW_EDGE_V)
    {
        player->pos.z += DIAMETER_V;
        player->pos_smooth.z += DIAMETER_V;
    }

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

    player->chunk = (v3i16){
            floorf((f32)player->pos_smooth.x / chunk_diameter),
            floorf((f32)player->pos_smooth.y / chunk_diameter),
            floorf((f32)player->pos_smooth.z / chunk_diameter),
        };

    if ((player->delta_chunk.x - player->chunk.x)
            || (player->delta_chunk.y - player->chunk.y)
            || (player->delta_chunk.z - player->chunk.z))
        flag |= FLAG_MAIN_CHUNK_BUF_DIRTY;

    static f32 zoom = 0.0f;
    if (lily.flag & FLAG_PLAYER_ZOOMER)
        zoom = player->camera.zoom;
    else zoom = 0.0f;

    if (!(player->flag & FLAG_PLAYER_CAN_JUMP) &&
            !(player->flag & FLAG_PLAYER_FLYING))
    {
        player->pos_lerp_speed.x =
            SET_LERP_SPEED_GLIDE;
        player->pos_lerp_speed.y =
            SET_LERP_SPEED_GLIDE;

        update_gravity(render, player);
    }

    if (player->flag & FLAG_PLAYER_FLYING)
    {
        player->flag &= ~(FLAG_PLAYER_CAN_JUMP | FLAG_PLAYER_FALLING);

        player->pos_lerp_speed.x =
            SET_LERP_SPEED_GLIDE;
        player->pos_lerp_speed.y =
            SET_LERP_SPEED_GLIDE;
        player->pos_lerp_speed.z =
            SET_LERP_SPEED_DEFAULT;

        player->vel = v3fzero;
        player->movement_speed =
            SET_PLAYER_SPEED_FLY * render->frame_delta;
        player->camera.fovy = 80.0f - zoom;
    }
    else player->flag |= FLAG_PLAYER_FALLING;

    if ((player->flag & FLAG_PLAYER_SNEAKING)
            && !(player->flag & FLAG_PLAYER_FLYING))
        player->movement_speed =
            SET_PLAYER_SPEED_SNEAK * render->frame_delta;
    else if (player->flag & FLAG_PLAYER_SPRINTING)
    {
        if (!(player->flag & FLAG_PLAYER_FLYING))
        {
            player->movement_speed =
                SET_PLAYER_SPEED_SPRINT * render->frame_delta;
            player->camera.fovy = 75.0f - zoom;
        }
        else
        {
            player->movement_speed =
                SET_PLAYER_SPEED_FLY_FAST * render->frame_delta;
            player->camera.fovy = 90.0f - zoom;
        }
    }
    else if (!(player->flag & FLAG_PLAYER_SNEAKING)
            && !(player->flag & FLAG_PLAYER_SPRINTING)
            && !(player->flag & FLAG_PLAYER_FLYING))
    {
        player->pos_lerp_speed.x =
            SET_LERP_SPEED_DEFAULT;
        player->pos_lerp_speed.y =
            SET_LERP_SPEED_DEFAULT;
        player->pos_lerp_speed.z =
            SET_LERP_SPEED_RIGID;

        player->movement_speed =
            SET_PLAYER_SPEED_WALK * render->frame_delta;
        player->camera.fovy = 70.0f - zoom;
    }

    player->camera.fovy =
        clamp_f32(player->camera.fovy, 1.0f, SET_FOV_MAX);
    player->camera.fovy_smooth =
        lerp_f32(player->camera.fovy_smooth,
                player->camera.fovy,
                settings.lerp_speed, render->frame_delta);
}

void
player_camera_movement_update(Render *render, Player *player,
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

    switch (player->perspective)
    {
        case MODE_CAMERA_1ST_PERSON:
            player->camera.pos =
                (v3f32){
                    player->pos_smooth.x,
                    player->pos_smooth.y,
                    player->pos_smooth.z + player->eye_height
                };
            break;

        case MODE_CAMERA_3RD_PERSON:
            player->camera.pos =
                (v3f32){
                    player->pos_smooth.x -
                        ((CYAW * CPCH) * player->camera_distance),
                    player->pos_smooth.y +
                        ((SYAW * CPCH) * player->camera_distance),
                    player->pos_smooth.z + player->eye_height +
                        (SPCH * player->camera_distance),
                };
            break;

        case MODE_CAMERA_3RD_PERSON_FRONT:
            player->camera.pos =
                (v3f32){
                    player->pos_smooth.x +
                        ((CYAW * CPCH) * player->camera_distance),
                    player->pos_smooth.y -
                        ((SYAW * CPCH) * player->camera_distance),
                    player->pos_smooth.z + player->eye_height -
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

    player->camera_hud.pos =        player->camera.pos;
    player->camera_hud.sin_pitch =  player->camera.sin_pitch;
    player->camera_hud.cos_pitch =  player->camera.cos_pitch;
    player->camera_hud.sin_yaw =    player->camera.sin_yaw;
    player->camera_hud.cos_yaw =    player->camera.cos_yaw;
}

void
player_target_update(Player *player)
{
    const f32 SPCH = player->sin_pitch;
    const f32 CPCH = player->cos_pitch;
    const f32 SYAW = player->sin_yaw;
    const f32 CYAW = player->cos_yaw;

    player->target =
    (v3f64){
        player->pos_smooth.x +
            ((CYAW * CPCH) * player->camera_distance),
        player->pos_smooth.y -
            ((SYAW * CPCH) * player->camera_distance),
        player->pos_smooth.z + player->eye_height -
            (SPCH * player->camera_distance),
    };

    player->delta_target =
        (v3i64){
            (i64)floorf(player->target.x),
            (i64)floorf(player->target.y),
            (i64)floorf(player->target.z),
        };
}

void
player_kill(Player *player)
{
    player->vel = v3fzero;
    player->mass = 0.0f;
    player->movement_speed = 0.0f;
    player->container_state = 0;
    player->flag = FLAG_PLAYER_DEAD;
}

void
player_respawn(Player *player)
{
    player->pos =
        (v3f64){
            player->spawn_point.x,
            player->spawn_point.y,
            player->spawn_point.z
        };
    player->flag = 0;
}

void
update_gravity(Render *render, Player *player)
{
    if (player->flag & FLAG_PLAYER_FALLING)
        player->vel.z += (GRAVITY * player->mass * render->frame_delta);
    player->pos.z += player->vel.z * render->frame_delta;
}

/* TODO: make AABB collision work */
void
player_collision_update(Player *player)
{
    player->collision_check_start = (v3f32){
            floorf(player->pos.x - (player->scale.x / 2.0f)) - 1.0f,
            floorf(player->pos.y - (player->scale.y / 2.0f)) - 1.0f,
            floorf(player->pos.z) - 1.0f,
        };

    player->collision_check_end = (v3f32){
            ceilf(player->scale.x) + 2.0f,
            ceilf(player->scale.y) + 2.0f,
            ceilf(player->scale.z) + 2.0f,
        };


#if 0 /* TODO: remove this parse collision feet stuff */
    Chunk *target_chunk = get_chunk(&player->lastPos, &player->flag, FLAG_PARSE_COLLISION_FEET);
    if (target_chunk->i
            [z - 1 - WORLD_BOTTOM]
            [y - (target_chunk->pos.y * CHUNK_SIZE)]
            [x - (target_chunk->pos.x * CHUNK_SIZE)] & NOT_EMPTY)
    {
        player->pos.z = ceilf(targetCoordinatesFeet->z) + WORLD_BOTTOM + 1;
        player->vel.z = 0;
        if (player->flag & FLAG_PLAYER_FLYING) player->flag &= ~FLAG_PLAYER_FLYING;
        player->flag |= FLAG_PLAYER_CAN_JUMP;
        player->flag &= ~FLAG_PLAYER_FALLING;
    } else player->flag |= FLAG_PLAYER_FALLING;

    // TODO: move to new 'void parse_camera_collisions()'
    player->camera_distance = SET_CAMERA_DISTANCE_MAX;
#endif
}

f64
get_time_ms(void)
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
