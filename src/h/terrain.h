#ifndef GAME_TERRAIN_H
#define GAME_TERRAIN_H

#include <engine/h/types.h>

extern f32 *RAND_TAB;

/* return non-zero on failure and '*GAME_ERR' is set accordingly */
u32 rand_init(void);

void rand_free(void);
v3f32 random_2d(i32 x, i32 y, u32 seed);
v3f32 random_3d(i32 x, i32 y, i32 z, u32 seed);
f32 gradient_2d(f32 vx, f32 vy, f32 ax, f32 ay);
f32 gradient_3d(f32 vx, f32 vy, f32 vz, f32 ax, f32 ay, f32 az);

/* coordinates = current block to sample */
f32 terrain_noise_2d(v3i32 coordinates, f32 amplitude, f32 frequency);

/* coordinates = current block to sample */
f32 terrain_noise_3d(v3i32 coordinates, f32 amplitude, f32 frequency);

/* basic terrain of mountains and valleys,
 * built on terrain_noise() */
b8 terrain_land(v3i32 coordinates);

#endif /* GAME_TERRAIN_H */
