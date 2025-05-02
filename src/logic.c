#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "h/logic.h"
#include "h/chunking.h"

// ---- variables --------------------------------------------------------------
Vector2 mouseDelta;

bool get_double_press(Player* player, KeyboardKey key) {
    static f64 doublePressStartTime = 0;
    static KeyboardKey doublePressKey = KEY_NULL;

    if (((player->state & STATE_DOUBLE_PRESS) && (f64)(get_time_ms() - doublePressStartTime >= 0.25f)) ||
            key != doublePressKey)
        player->state &= ~STATE_DOUBLE_PRESS;
    else {
        doublePressKey = KEY_NULL;
        return true;
    }

    if (!(player->state & STATE_DOUBLE_PRESS)) {
        player->state |= STATE_DOUBLE_PRESS;
        doublePressKey = key;
        doublePressStartTime = get_time_ms();
    }
    return false;
}

void update_player_states(Player* player) {
    player->chunk = (v2i16){
            ((i32)floorf((f32)player->pos.x / CHUNK_DIAMETER)) * CHUNK_DIAMETER,
            ((i32)floorf((f32)player->pos.y / CHUNK_DIAMETER)) * CHUNK_DIAMETER,
        };

    if (!(player->state & STATE_CAN_JUMP) && !(player->state & STATE_FLYING))
        give_gravity(player);

    if (player->state & STATE_FLYING) {
        player->v = v3fzero;
        player->movementSpeed = PLAYER_SPEED_FLY * dt;
        player->camera.fovy = 80; //TODO: revise (add lerp)
    }

    if (player->state & STATE_SNEAKING && !(player->state & STATE_FLYING))
        player->movementSpeed = PLAYER_SPEED_SNEAK * dt;
    else if (player->state & STATE_SPRINTING) {
        if (!(player->state & STATE_FLYING)) {
            player->movementSpeed = PLAYER_SPEED_SPRINT * dt;
            player->camera.fovy = 75;
        } else {
            player->movementSpeed = PLAYER_SPEED_FLY_FAST * dt;
            player->camera.fovy = 90;
        }
    } else if (!(player->state & STATE_SNEAKING) && !(player->state & STATE_SPRINTING) && !(player->state & STATE_FLYING)) {
        player->movementSpeed = PLAYER_SPEED_WALK * dt;
        player->camera.fovy = 70;
    }

    //TODO: revise (actually kill the player)
    if (player->pos.z < WORLD_KILL_Z) {
        player->pos = (Vector3){
                player->spawn_point.x,
                player->spawn_point.y,
                player->spawn_point.z,
            };
        player->v.z = 0;
    }
}

void update_camera_movements_player(Player* player) {
    if (!(stateMenuDepth) && !(state & STATE_SUPER_DEBUG)) {
        player->yaw -= (f32)mouseDelta.x * setting.mouseSensitivity;
        player->pitch -= (f32)mouseDelta.y * setting.mouseSensitivity;
    }

    player->yaw = fmodf(player->yaw, 360.0f);
    if (player->yaw < 0.0f) player->yaw += 360.0f;
    if (player->pitch > 90.0f) player->pitch = 90.0f;
    else if (player->pitch < -90.0f) player->pitch = -90.0f;

    player->sinPitch =  sinf(player->pitch * MC_C_DEG2RAD);
    player->cosPitch =  cosf(player->pitch * MC_C_DEG2RAD);
    player->sinYaw =    sinf(player->yaw * MC_C_DEG2RAD);
    player->cosYaw =    cosf(player->yaw * MC_C_DEG2RAD);

    Vector3 playerCameraUp;
    playerCameraUp = (Vector3){
            -player->cosYaw * player->sinPitch,
            -player->sinYaw * player->sinPitch,
            player->cosPitch,
        };

    switch (player->perspective) {
        case 0: // ---- 1st person ---------------------------------------------
            player->camera.position = (Vector3){
                player->pos.x, player->pos.y, player->pos.z + player->eyeHeight,
                };
            player->camera.target = (Vector3){
                    player->pos.x + ((player->cosYaw * player->cosPitch) * setting.reachDistance),
                    player->pos.y + ((player->sinYaw * player->cosPitch) * setting.reachDistance),
                    player->camera.position.z + (player->sinPitch * setting.reachDistance),
                };
            player->camera.up = playerCameraUp;
            break;

        case 1: // ---- 3rd person back ----------------------------------------
            player->camera.position = (Vector3){
                    player->pos.x - ((player->cosYaw * player->cosPitch) * player->cameraDistance),
                    player->pos.y - ((player->sinYaw * player->cosPitch) * player->cameraDistance),
                    player->pos.z + player->eyeHeight - (player->sinPitch * player->cameraDistance),
                };
            player->camera.target = (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + player->eyeHeight,
                };
            player->camera.up = playerCameraUp;
            break;

        case 2: // ---- 3rd person front ---------------------------------------
            player->camera.position = (Vector3){
                    player->pos.x + ((player->cosYaw * player->cosPitch) * player->cameraDistance),
                    player->pos.y + ((player->sinYaw * player->cosPitch) * player->cameraDistance),
                    player->pos.z + player->eyeHeight + (player->sinPitch * player->cameraDistance),
                };
            player->camera.target = (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + player->eyeHeight,
                };
            player->camera.up = playerCameraUp;
            break;

            // TODO: make the stalker camera mode
        case 3: // ---- stalker ------------------------------------------------
            player->camera.target = (Vector3){
                    player->pos.x, player->pos.y, player->pos.z + player->eyeHeight,
                };
            break;

            // TODO: make the spectator camera mode
        case 4: // ---- spectator ----------------------------------------------
            break;
    }
}

void update_camera_movements_debug_info(Camera3D* camera, Player* player) {
    if (player->perspective == 0 || player->perspective == 1) {
        camera->position = (Vector3){
                -cosf(player->yaw * MC_C_DEG2RAD) * cosf(player->pitch * MC_C_DEG2RAD),
                -sinf(player->yaw * MC_C_DEG2RAD) * cosf(player->pitch * MC_C_DEG2RAD),
                -sinf(player->pitch * MC_C_DEG2RAD),
            };
        camera->up = (Vector3){
                -cosf(player->yaw * MC_C_DEG2RAD) * sinf(player->pitch * MC_C_DEG2RAD),
                -sinf(player->yaw * MC_C_DEG2RAD) * sinf(player->pitch * MC_C_DEG2RAD),
                cosf(player->pitch * MC_C_DEG2RAD),
            };
        camera->target = Vector3Zero();
    } else if (player->perspective == 2) {
        camera->position = (Vector3){
                cosf(player->yaw * MC_C_DEG2RAD) * cosf(player->pitch * MC_C_DEG2RAD),
                sinf(player->yaw * MC_C_DEG2RAD) * cosf(player->pitch * MC_C_DEG2RAD),
                sinf(player->pitch * MC_C_DEG2RAD),
            };
        camera->target = Vector3Zero();
        camera->up = (Vector3){
                -cosf(player->yaw * MC_C_DEG2RAD) * sinf(player->pitch * MC_C_DEG2RAD),
                -sinf(player->yaw * MC_C_DEG2RAD) * sinf(player->pitch * MC_C_DEG2RAD),
                cosf(player->pitch * MC_C_DEG2RAD),
            };
    }
}

void kill_player(Player* player) {
    *player = (Player){
            .v = {0.0f},
            .m = 0.0f,
            .movementSpeed = 0.0f,
            .containerState = 0,
            .state = STATE_DEAD,
    };
}

void player_respawn(Player* player) {
    *player = (Player){
            .pos = (Vector3){
                player->spawn_point.x,
                player->spawn_point.y,
                player->spawn_point.z},
            .state = 0,
    };
}

bool check_delta_target(Vector3* coordinates, v3i32* delta_target) {
    if (delta_target->x == floorf(coordinates->x) &&
            delta_target->y == floorf(coordinates->y) &&
            delta_target->z == floorf(coordinates->z))
        return false;

    delta_target->x = (i32)floorf(coordinates->x);
    delta_target->y = (i32)floorf(coordinates->y);
    delta_target->z = (i32)floorf(coordinates->z);
    return true;
}

bool is_range_within_ff(f32* pos, f32 start, f32 end) {
    if (*pos < start || *pos > end) return false;
    return true;
}

bool is_range_within_v2ff(v2f32* pos, v2f32 start, v2f32 end) {
    if (pos->x < start.x || pos->x > end.x ||
            pos->y < start.y || pos->y > end.y) return false;
    return true;
}

bool is_range_within_v3fi(Vector3* pos, v3i32 start, v3i32 end) {
    if (pos->x < start.x || pos->x > end.x ||
            pos->y < start.y || pos->y > end.y ||
            pos->z < start.z || pos->z > end.z) return false;
    return true;
}

b8 is_ray_intersect(Player* player) { //TODO: make the player ray intersection
    if (targetChunk->i[player->delta_target.z][player->delta_target.y][player->delta_target.x])
        return true;
    return false;
}

void give_gravity(Player* player) { // TODO: fix player gravity
    if (player->state & STATE_FALLING)
        player->v.z -= (PLAYER_JUMP_HEIGHT * GRAVITY * player->m * dt);
    player->pos.z += player->v.z;
    //printf("   previous time: %.2lf   delta time: %.2lf\n", get_time_ms(), get_delta_time(&start_time)); //temp
}

/*
void update_collision_static(Player* player) { // TODO: make AABB collision work
    player->collisionCheckStart = (Vector3){
            floorf(player->pos.x - (player->scl.x / 2.0f)) - 1,
            floorf(player->pos.y - (player->scl.y / 2.0f)) - 1,
            floorf(player->pos.z) - 1,
        };

    player->collisionCheckEnd = (Vector3){
            ceilf(player->scl.x) + 2,
            ceilf(player->scl.y) + 2,
            ceilf(player->scl.z) + 2,
        };

    Chunk* targetChunk = get_chunk(&player->lastPos, &player->state, STATE_PARSE_COLLISION_FEET);
    if ((player->state & STATE_PARSE_COLLISION_FEET) && player->pos.z > WORLD_BOTTOM) {
        if (targetChunk->i
                [z - 1 - WORLD_BOTTOM]
                [y - (targetChunk->pos.y * CHUNK_SIZE)]
                [x - (targetChunk->pos.x * CHUNK_SIZE)] & NOT_EMPTY) {
            player->pos.z = ceilf(targetCoordinatesFeet->z) + WORLD_BOTTOM + 1;
            player->v.z = 0;
            if (player->state & STATE_FLYING) player->state &= ~STATE_FLYING;
            player->state |= STATE_CAN_JUMP;
            player->state &= ~STATE_FALLING;
        } else player->state |= STATE_FALLING;
    }
    //TODO: move to new 'void parse_camera_collisions()'
    player->cameraDistance = SETTING_CAMERA_DISTANCE_MAX;
}
*/

f64 get_time_ms() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec + (f64)tp.tv_usec / 1000000.0f;
}

bool get_timer(f64* timeStart, f32 interval) {
    if (get_time_ms() - *timeStart >= interval) {
        *timeStart = get_time_ms();
        return true;
    }
    return false;
}

void draw_default_grid(Color x, Color y, Color z) {
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

