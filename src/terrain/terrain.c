#include "deps/fossil/common/limits.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/math/noise.h"
#include "deps/fossil/math/vector.h"

#include "../chunking/chunking.h"

#include "../h/world.h"

#include "terrain.h"

#include <stdio.h>
#include <math.h>

#define TERRAIN_SEED_DEFAULT 0

/* terrain shape */
#define FREQ_CONTINENTAL    (1.0f / 1033.0f)
#define FREQ_REGIONAL       (1.0f / 250.0f)
#define FREQ_LOCAL          (1.0f / 109.0f)
#define FREQ_DETAIL         (1.0f / 16.0f)

/* terrain modifiers (and/or biome selection) */
#define FREQ_TEMPERATURE    (1.0f / 326.0f)
#define FREQ_HUMIDITY       (1.0f / 726.0f)
#define FREQ_EXTREMITY      (1.0f / 953.34f)
#define FREQ_ROUGHNESS      (1.0f / 368.2f)
#define FREQ_LIFE           (1.0f / 443.04f)

#define TERRAIN_SEA_LEVEL           0
#define TERRAIN_CAVE_LEVEL          ((WORLD_RADIUS_VERTICAL / 2) * CHUNK_DIAMETER)
#define TERRAIN_SQUISH_MAGNITUDE    0.03f

static hhc_biome biome_buf[BIOME_COUNT] = {0};

static hhc_biome biome_init(str *name, f32 temperature, f32 humidity, f32 extremity,
    f32 roughness, f32 depth, f32 life)
{
    hhc_biome biome = {0};
    static u32 seed = 4911577;
    f64 sum = 0.0;
    u32 i = 0;

    snprintf(biome.name, FSL_ID_CAP, "%s", name);

    for (i = 0; i < BIOME_PARAM_COUNT; ++i)
    {
        biome.param[i] = (f64)fsl_rand_u32(seed++) / (f64)FSL_U32_MAX;
        sum += biome.param[i];
    }
    for (i = 0; i < BIOME_PARAM_COUNT; ++i)
        biome.param[i] /= sum;

    biome.temperature = temperature;
    biome.humidity = humidity;
    biome.extremity = extremity;
    biome.roughness = roughness;
    biome.depth = depth;
    biome.life = life;

    return biome;
}

f32 biome_score_get(hhc_biome a, hhc_biome b)
{
    i32 i = 0;
    f32 sum = 0.0f;
    for (; i < BIOME_PARAM_COUNT; ++i)
    {
        b.param[i] -= a.param[i];
        b.param[i] *= b.param[i];
        sum += b.param[i];
    }
    return sum;
}

void terrain_init(void)
{
    biome_buf[BIOME_STONE] = biome_init("Stone",
            0.350f, 0.000f, 0.334f, 0.300f, 0.000f, 0.000f);

    biome_buf[BIOME_HILLS] = biome_init("Hills",
            0.390f, 0.100f, 0.600f, 0.500f, 70.000f, 50.000f);

    biome_buf[BIOME_SANDSTORM] = biome_init("Sandstorm",
            0.557f, 0.000f, 0.100f, 0.200f, 0.000f, 0.300f);

    biome_buf[BIOME_DECAYING_LANDS] = biome_init("Decaying Lands",
            0.163f, 0.040f, 0.900f, 0.953f, 0.000f, 7.000f);

    biome_buf[BIOME_JUNGLE] = biome_init("Jungle",
            0.270f, 0.290f, 0.430f, 0.780f, 3.000f, 260.000f);
}

hhc_terrain_noise terrain_noise_make(v3i32 coordinates)
{
    hhc_terrain_noise noise = {0};
    f32 x = (f32)(coordinates.x + 1324);
    f32 y = (f32)(coordinates.y - 3272);
    f32 z = (f32)(coordinates.z - 50);
    f32 coef = 2.0f / 3.0f;

    noise.continental = fsl_perlin_noise_2d(x, y, 1000.0f, FREQ_CONTINENTAL, world.seed);
    noise.regional = fsl_perlin_noise_2d(x, y, 200.0f, FREQ_REGIONAL, world.seed + 10);
    noise.local = fsl_perlin_noise_2d(x, y, 50.0f, FREQ_LOCAL, world.seed + 20);
    noise.detail = fsl_perlin_noise_2d(x, y, 5.0f, FREQ_DETAIL, world.seed + 30);

    noise.temperature = fsl_perlin_noise_2d(x, y, coef, FREQ_TEMPERATURE, world.seed + 40) + coef;
    noise.humidity = fsl_perlin_noise_2d(x, y, coef, FREQ_HUMIDITY, world.seed + 50) + coef;
    noise.extremity = fsl_perlin_noise_2d(x, y, coef, FREQ_EXTREMITY, world.seed + 60) + coef;
    noise.roughness = fsl_perlin_noise_2d(x, y, coef, FREQ_ROUGHNESS, world.seed + 70) + coef;
    noise.life = fsl_perlin_noise_2d(x, y, coef, FREQ_LIFE, world.seed + 80) + coef;

    noise.temperature = fsl_clamp_f32(noise.temperature, 0.0f, 1.0f);
    noise.humidity = fsl_clamp_f32(noise.humidity, 0.0f, 1.0f);
    noise.extremity = fsl_clamp_f32(noise.extremity, 0.0f, 1.0f);
    noise.roughness = fsl_clamp_f32(noise.roughness, 0.0f, 1.0f);
    noise.life = fsl_clamp_f32(noise.life, 0.0f, 1.0f);

    noise.cost = CHUNK_WORK_COST_GENERATE_NON_AIR;
    return noise;
}

hhc_terrain_noise terrain_noise_lerp(const hhc_terrain_noise *a, const hhc_terrain_noise *b, f32 t)
{
    hhc_terrain_noise noise = {0};
    t = t * t * t * (t * (6.0f * t - 15.0f) + 10.0f);

    noise.continental = a->continental + (b->continental - a->continental) * t;
    noise.regional = a->regional + (b->regional - a->regional) * t;
    noise.local = a->local + (b->local - a->local) * t;
    noise.detail = a->detail + (b->detail - a->detail) * t;

    noise.temperature = a->temperature + (b->temperature - a->temperature) * t;
    noise.humidity = a->humidity + (b->humidity - a->humidity) * t;
    noise.extremity = a->extremity + (b->extremity - a->extremity) * t;
    noise.roughness = a->roughness + (b->roughness - a->roughness) * t;
    noise.life = a->life + (b->life - a->life) * t;

    noise.cost = CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE;
    return noise;
}

hhc_terrain_noise terrain_noise_bilerp(
        const hhc_terrain_noise *a, const hhc_terrain_noise *b,
        const hhc_terrain_noise *c, const hhc_terrain_noise *d,
        f32 tx, f32 ty)
{
    hhc_terrain_noise noise[2] = {0};
    chunk_work_cost cost = 0;

    noise[0] = terrain_noise_lerp(a, b, tx);
    cost += noise[0].cost;

    noise[1] = terrain_noise_lerp(c, d, tx);
    cost += noise[1].cost;

    noise[0] = terrain_noise_lerp(&noise[0], &noise[1], ty);
    noise[0].cost += cost;

    return noise[0];
}

hhc_terrain terrain_shape(v3i32 coordinates, const hhc_terrain_noise *noise)
{
    hhc_terrain terrain = {0};
    hhc_biome biome = {0};
    f32 biome_score[BIOME_COUNT] = {0};
    f32 biome_best_score = 10000.0f;
    i32 biome_best_index = 0;
    i32 i = BIOME_COUNT;

    biome.temperature = noise->temperature;
    biome.humidity = noise->humidity;
    biome.extremity = noise->extremity;
    biome.roughness = noise->roughness;
    biome.life = noise->life;

    biome.param[BIOME_PARAM_TEMPERATURE] = noise->temperature;
    biome.param[BIOME_PARAM_HUMIDITY] = noise->humidity;
    biome.param[BIOME_PARAM_EXTREMITY] = noise->extremity;
    biome.param[BIOME_PARAM_ROUGHNESS] = noise->roughness;
    biome.param[BIOME_PARAM_LIFE] = noise->life;

    while (i--)
    {
        biome_score[i] = biome_score_get(biome, biome_buf[i]);
        if (biome_score[i] < biome_best_score)
        {
            biome_best_score = biome_score[i];
            biome_best_index = i;
        }
    }

    terrain.value =
        ((noise->continental + noise->regional) * (1.0f - noise->temperature) +
        noise->local) * noise->extremity +
        noise->detail * noise->roughness;

    if ((f32)coordinates.z < terrain.value)
    {
        terrain.block_id = biome_best_index + 1;
    }

    return terrain;
}
