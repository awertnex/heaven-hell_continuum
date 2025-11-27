#ifndef GAME_TERRAIN_H
#define GAME_TERRAIN_H

/* coordinates = current block to sample */
f32 terrain_noise(v3i32 coordinates, f32 amplitude, f32 frequency);

/* basic terrain of mountains and valleys,
 * built on terrain_noise() */
f32 terrain_land(v3i32 coordinates);

#endif /* GAME_TERRAIN_H */
