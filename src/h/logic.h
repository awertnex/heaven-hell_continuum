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

/*! @brief get time for logic purposes, like for random number generators
 *  that require a seed.
 *
 *  get an unsigned 64-bit integer value based on unix time,
 *  but added nanoseconds of unix time.
 *
 *  @remark not accurate to actual time.
 */
u64 get_time_logic(void);

/*! @brief get time since process started in seconds and fractional milliseconds.
 */
f64 get_time_f64(void);

b8 get_timer(f64 *time_start, f32 interval);

/*! @brief update player parameters like movement and zoom based on flags.
 *
 *  1. wrap player coordinates, teleport to the other side of the world if
 *     they cross a world edge.
 *  2. calculate 'p->chunk' from 'p->pos'.
 *  3. add 'FLAG_MAIN_CHUNK_BUF_DIRTY' to 'flag' if player crosses a chunk boundary.
 *  4. update 'p->movement_speed' based on 'p->flag'.
 *  5. update various movement permissions and mechanics based on 'p->flag'.
 *  6. update 'p->camera.fovy' based on 'p->flag' and 'p->camera.zoom'.
 *  7. update player gravity.
 *  8. add vector 'p->movement' to 'p->pos'.
 */
void player_state_update(Player *p, f64 dt);

/*! @brief update player chunk deltas.
 *
 *  calculate current chunk and delta chunk, and determine whether
 *  FLAG_MAIN_CHUNK_BUF_DIRTY should be set or not.
 */
void player_chunk_update(Player *p);

/*! @brief calculate camera rotations and mechanics based on 'player->camera_mode'.
 *
 *  @param use_mouse = let mouse delta move the camera, useful for interacting
 *  with UI instead of player.
 */
void player_camera_movement_update(Player *p, v2f64 mouse_delta, b8 use_mouse);

void player_target_update(Player *p);
void set_player_pos(Player *p, f64 x, f64 y, f64 z);
void set_player_block(Player *p, i64 x, i64 y, i64 z);
void set_player_spawn(Player *p, i64 x, i64 y, i64 z);
void player_kill(Player *p);

/*! @brief re-spawn player.
 *
 *  @param hard = TRUE will reset all player stats, FALSE will only teleport.
 */
void player_spawn(Player *p, b8 hard);

void player_collision_update(Player *p, f64 dt);

/*! @brief make a broad-phase region for collision detection.
 *
 *  useful for limiting collision checks to only within the specified region.
 *
 *  @param padding = padding difference between the region and the bounding box
 *  it encapsulates.
 */
BoundingBox make_aabb_broad_phase_region(Player *p, f32 padding, f64 dt);

b8 is_aabb_intersect(v3f64 a[2], v3f64 b[2]);

/*! @brief get unit scalar of collision time between 'a' and 'b'.
 *
 *  @param velocity = velocity of 'a' since this function assumes 'b' is static.
 *  @param diff = vector for collision resolution.
 *
 *  @return unit scalar of difference between entry and exit times of collision.
 *  @return 1.0f on failure.
 */
f32 get_aabb_collision_time(v3f64 a[2], v3f64 b[2], v3f32 velocity, v3f32 *diff, v3f32 *normal);

void gravity_update(v3f64 *movement, v3f32 *influence, f32 weight, f64 dt);

#endif /* GAME_LOGIC_H */
