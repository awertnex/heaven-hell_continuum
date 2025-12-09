#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <engine/h/core.h>
#include "main.h"

enum ContainerState
{
    STATE_CONTR_CHEST,
    STATE_CONTR_CRAFTING_TABLE,
    STATE_CONTR_FURNACE,
    STATE_CONTR_INVENTORY_SURVIVAL,
    STATE_CONTR_INVENTORY_SANDBOX,
    STATE_CONTR_COUNT,
}; /* ContainerState */

f64 get_time_ms(void);
b8 get_timer(f64 *time_start, f32 interval);

/*! @brief update player parameters like movement and zoom based on flags.
 *
 *  1. wrap player coordinates, teleport to the other side of the world if
 *     they cross a world edge.
 *  2. calculate 'player->chunk' from 'player->pos'.
 *  3. add 'FLAG_MAIN_CHUNK_BUF_DIRTY' to 'flag' if player crosses a chunk boundary.
 *  4. update 'player->movement_speed' based on 'player->flag'.
 *  5. update various movement permissions and mechanics based on 'player->flag'.
 *  6. update 'player->camera.fovy' based on 'player->flag' and 'player->camera.zoom'.
 *  7. update player gravity.
 *  8. add vector 'player->movement' to 'player->pos'.
 */
void player_state_update(f64 dt, Player *player, u64 chunk_diameter,
        u64 radius, u64 radius_v, u64 diameter, u64 diameter_v);

/*! @brief calculate camera rotations and mechanics based on 'player->camera_mode'.
 *
 *  @param use_mouse = let mouse delta move the camera.
 *
 *  @remark 'use_mouse' is turned off for when interacting with ui instead of
 *  player camera.
 */
void player_camera_movement_update(v2f64 mouse_delta, Player *player, b8 use_mouse);

void player_target_update(Player *player);
void set_player_pos(Player *player, f64 x, f64 y, f64 z);
void set_player_block(Player *player, i64 x, i64 y, i64 z);
void set_player_spawn(Player *player, i64 x, i64 y, i64 z);
void player_kill(Player *player);
void player_respawn(Player *player);
void player_collision_update(f64 dt, Player *player);
b8 is_intersect_aabb(v3f64 box_1[2], v3f64 box_1_last[2], v3f64 box_2[2]);
void gravity_update(f64 dt, v3f64 *position, v3f32 *acceleration, f32 weight);

#endif /* GAME_LOGIC_H */
