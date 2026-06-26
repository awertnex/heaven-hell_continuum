#ifndef HHC_TERRAIN_H
#define HHC_TERRAIN_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/common/limits.h"

#include "../chunking/chunk_work.h"

#include "../h/assets.h"

#include "noise.h"

enum hhc_biome_index
{
    BIOME_STONE,
    BIOME_HILLS,
    BIOME_SANDSTORM,
    BIOME_DECAYING_LANDS,
    BIOME_JUNGLE,
    BIOME_COUNT
}; /* hhc_biome_index */

enum hhc_biome_param_index
{
    BIOME_PARAM_TEMPERATURE,
    BIOME_PARAM_HUMIDITY,
    BIOME_PARAM_EXTREMITY,
    BIOME_PARAM_ROUGHNESS,
    BIOME_PARAM_DEPTH,
    BIOME_PARAM_LIFE,
    BIOME_PARAM_COUNT
}; /* hhc_biome_param_index */

typedef struct hhc_biome
{
    str name[FSL_ID_CAP];
    f32 param[BIOME_PARAM_COUNT];
    f32 temperature;
    f32 humidity;
    f32 extremity;  /* big detail height */
    f32 roughness;  /* small detail height */
    f32 depth;
    f32 life;
} hhc_biome;

typedef struct hhc_terrain_noise
{
    hhc_noise_sample continental;
    hhc_noise_sample regional;
    hhc_noise_sample local;
    hhc_noise_sample detail;

    hhc_noise_sample temperature;
    hhc_noise_sample humidity;
    hhc_noise_sample extremity;
    hhc_noise_sample roughness;
    hhc_noise_sample life;
} hhc_terrain_noise;

typedef struct hhc_terrain
{
    enum hhc_biome_index biome;
    enum block_id block_id;

    /*!
     *  @brief value/density of block at generation position.
     */
    f32 value;
} hhc_terrain;

/*!
 *  @brief initialize default terrain settings and biome parameters.
 */
void terrain_init(void);

/*!
 *  @brief initialize a single axis of pre-defined 2D terrain noises.
 *
 *  @param x position along specified axis.
 */
chunk_work_cost terrain_noise_axis_init_2d(hhc_terrain_noise *noise, u32 axis, f32 x);

/*!
 *  @brief initialize a single axis of pre-defined 3D terrain noises.
 *
 *  @param x position along specified axis.
 */
chunk_work_cost terrain_noise_axis_init_3d(hhc_terrain_noise *noise, u32 axis, f32 x);

/*!
 *  @brief finalize and bake all pre-defined 2D terrain noises.
 */
chunk_work_cost terrain_noise_make_2d(hhc_terrain_noise *noise);

/*!
 *  @brief make base noise maps for terrain.
 */
chunk_work_cost terrain_noise_make(hhc_terrain_noise *noise, f32 x, f32 y, f32 z);

/*!
 *  @brief interpolate all noises in `a` and `b` using linear interpolation.
 */
chunk_work_cost terrain_noise_lerp(hhc_terrain_noise *dst,
        const hhc_terrain_noise *a, const hhc_terrain_noise *b, f32 t);

/*!
 *  @brief interpolate all noises in `a`, `b`, `c` and `d` using bi-linear interpolation.
 */
chunk_work_cost terrain_noise_bilerp(hhc_terrain_noise *dst,
        const hhc_terrain_noise *a, const hhc_terrain_noise *b,
        const hhc_terrain_noise *c, const hhc_terrain_noise *d,
        f32 tx, f32 ty);

/*!
 *  @brief interpolate all noises in `a`, `b`, `c`, `d`, `e`, `f`, `g` and `h`
 *  using tri-linear interpolation.
 */
chunk_work_cost terrain_noise_trilerp(hhc_terrain_noise *dst,
        const hhc_terrain_noise *a, const hhc_terrain_noise *b,
        const hhc_terrain_noise *c, const hhc_terrain_noise *d,
        const hhc_terrain_noise *e, const hhc_terrain_noise *f,
        const hhc_terrain_noise *g, const hhc_terrain_noise *h,
        f32 tx, f32 ty, f32 tz);

/*!
 *  @brief default terrain shape.
 *
 *  @return terrain info (e.g., biome) from specified coordinates and noises.
 */
hhc_terrain terrain_shape(i32 x, i32 y, i32 z, const hhc_terrain_noise *noise);

/*!
 *  @brief compare likelihood of biomes `a` and `b` matching.
 *
 *  euclidean distance between relevant parameters is used.
 *
 *  @return biome score.
 */
f32 biome_score_get(hhc_biome a, hhc_biome b);

/*!
 *  @brief first attempt at terrain generation.
 *
 *  @return terrain info (e.g., block ID at specified coordinates).
 */
hhc_terrain terrain_land(v3i32 coordinates);

/*!
 *  @brief second attempt at terrain generation.
 *
 *  @return terrain info (e.g., block ID at specified coordinates).
 */
hhc_terrain terrain_decaying_lands(v3i32 coordinates);

/*!
 *  @brief third attempt at terrain generation.
 *
 *  @return terrain info (e.g., block ID at specified coordinates).
 */
hhc_terrain terrain_biome_blend_test(v3i32 coordinates);

#endif /* HHC_TERRAIN_H */
