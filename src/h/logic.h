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

/*! @brief update everything related to a player.
 *
 *  1. update kinematics and movement permissions based on 'p->flag'.
 *  2. update player FOV based on speed and camera zoom.
 *  3. update player collision.
 *  4. wrap player coordinates (teleport to the other side of the world if
 *    they cross a world edge).
 *  5. update player current chunk.
 *  6. make entire chunk buffer dirty if player crosses a chunk boundary.
 */
void player_update(Player *p, f64 dt);

/*! @brief update player chunk deltas.
 *
 *  calculate current chunk and delta chunk, and determine whether
 *  'FLAG_MAIN_CHUNK_BUF_DIRTY' should be set or not.
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

/*! @brief set player spawn point.
 */
void set_player_spawn(Player *p, i64 x, i64 y, i64 z);

/*! @brief re-spawn player.
 *
 *  @param hard = TRUE will reset all player stats, FALSE will only teleport to spawn.
 */
void player_spawn(Player *p, b8 hard);

void player_kill(Player *p);

b8 player_collision_update(Player *p, f64 dt);

/*! @brief make a collision capsule for collision detection.
 *
 *  useful for limiting collision checks to only within the capsule.
 *
 *  @param chunk = current chunk the player is in.
 *
 *  @param padding = padding difference between the capsule and the bounding box
 *  it encapsulates.
 *
 *  @remark collision capsule position is in chunk-relative coordinates, not world
 *  coordinates.
 */
CollisionCapsule make_collision_capsule(BoundingBox b, v3i32 chunk, v3f32 velocity, f32 padding, f64 dt);

b8 is_intersect_aabb(BoundingBox a, BoundingBox b);

/*! @brief get collision status and stats between 'a' and 'b' using
 *  the 'Swept AABB' algorithm.
 *
 *  @param velocity = velocity of 'a' since this function assumes 'b' is static.
 *
 *  @return entry time.
 */
f32 get_swept_aabb(BoundingBox a, BoundingBox b, v3f32 velocity, v3f32 *normal);

#endif /* GAME_LOGIC_H */
