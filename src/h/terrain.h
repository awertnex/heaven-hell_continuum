#ifndef GAME_TERRAIN_H
#define GAME_TERRAIN_H

#include <engine/h/types.h>

/*! @brief random number look-up table.
 *
 *  the values are extremely small so to directly use in terrain functions.
 *
 *  @remark read-only, initialized internally in 'rand_init()'.
 */
extern f32 *RAND_TAB;

/*! @brief initialize and allocate resources for 'RAND_TAB'.
 *
 *  allocate resources for 'RAND_TAB' and load its look-up from disk if found
 *  and build it if not found.
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 rand_init(void);

void rand_free(void);
v3f32 random_2d(i32 x, i32 y, u64 seed);
v3f32 random_3d(i32 x, i32 y, i32 z, u64 seed);
f32 gradient_2d(f32 vx, f32 vy, f32 ax, f32 ay);
f32 gradient_3d(f32 vx, f32 vy, f32 vz, f32 ax, f32 ay, f32 az);

/*  @param coordinates = current block to sample.
 */
f32 terrain_noise_2d(v3i32 coordinates, f32 amplitude, f32 frequency);

/*  @param coordinates = current block to sample.
 */
f32 terrain_noise_3d(v3i32 coordinates, f32 amplitude, f32 frequency);

/*! @brief basic terrain of mountains and valleys, built on 'terrain_noise()'
 *
 *  @return TRUE if result is greater than 'coordinates.z'.
 */
b8 terrain_land(v3i32 coordinates);

#endif /* GAME_TERRAIN_H */
