#include "deps/fossil/common/limits.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/math/noise.h"

#include "../chunking/chunking.h"

#include "../h/world.h"

#include "terrain.h"

#include <stdio.h>

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

chunk_work_cost terrain_noise_axis_init_2d(hhc_terrain_noise *noise, u32 axis, f32 x)
{
    chunk_work_cost cost = 0;

    cost += noise_sample_axis_init(&noise->continental, axis, x, FREQ_CONTINENTAL);
    cost += noise_sample_axis_init(&noise->regional, axis, x, FREQ_REGIONAL);
    cost += noise_sample_axis_init(&noise->local, axis, x, FREQ_LOCAL);
    cost += noise_sample_axis_init(&noise->detail, axis, x, FREQ_DETAIL);

    cost += noise_sample_axis_init(&noise->temperature, axis, x, FREQ_TEMPERATURE);
    cost += noise_sample_axis_init(&noise->humidity, axis, x, FREQ_HUMIDITY);
    cost += noise_sample_axis_init(&noise->extremity, axis, x, FREQ_EXTREMITY);
    cost += noise_sample_axis_init(&noise->roughness, axis, x, FREQ_ROUGHNESS);
    cost += noise_sample_axis_init(&noise->life, axis, x, FREQ_LIFE);
    return cost;
}

chunk_work_cost terrain_noise_axis_init_3d(hhc_terrain_noise *noise, u32 axis, f32 x)
{
    chunk_work_cost cost = 0;

    return cost;
}

chunk_work_cost terrain_noise_make_2d(hhc_terrain_noise *noise)
{
    chunk_work_cost cost = 0;
    f32 coef = 2.0f / 3.0f;

    cost += noise_sample_make_2d(&noise->continental, 1000.0f, world.seed);
    cost += noise_sample_make_2d(&noise->regional, 200.0f, world.seed + 10);
    cost += noise_sample_make_2d(&noise->local, 50.0f, world.seed + 20);
    cost += noise_sample_make_2d(&noise->detail, 5.0f, world.seed + 30);

    cost += noise_sample_make_2d(&noise->temperature, coef, world.seed + 40);
    cost += noise_sample_make_2d(&noise->humidity, coef, world.seed + 50);
    cost += noise_sample_make_2d(&noise->extremity, coef, world.seed + 60);
    cost += noise_sample_make_2d(&noise->roughness, coef, world.seed + 70);
    cost += noise_sample_make_2d(&noise->life, coef, world.seed + 80);

    noise->temperature.n = fsl_clamp_f32(noise->temperature.n + coef, 0.0f, 1.0f);
    noise->humidity.n = fsl_clamp_f32(noise->humidity.n + coef, 0.0f, 1.0f);
    noise->extremity.n = fsl_clamp_f32(noise->extremity.n + coef, 0.0f, 1.0f);
    noise->roughness.n = fsl_clamp_f32(noise->roughness.n + coef, 0.0f, 1.0f);
    noise->life.n = fsl_clamp_f32(noise->life.n + coef, 0.0f, 1.0f);

    return cost;
}

chunk_work_cost terrain_noise_make(hhc_terrain_noise *noise, f32 x, f32 y, f32 z)
{
    f32 coef = 0.6f;

    x += 1324;
    y -= 3272;
    z -= 50;

    noise->continental.n = fsl_perlin_noise_2d(x, y, 1000.0f, FREQ_CONTINENTAL, world.seed);
    noise->regional.n = fsl_perlin_noise_2d(x, y, 200.0f, FREQ_REGIONAL, world.seed + 10);
    noise->local.n = fsl_perlin_noise_2d(x, y, 50.0f, FREQ_LOCAL * 3.0f, world.seed + 20);
    noise->detail.n = fsl_perlin_noise_2d(x, y, 5.0f, FREQ_DETAIL, world.seed + 30);

    noise->temperature.n = fsl_perlin_noise_2d(x, y, coef, FREQ_TEMPERATURE, world.seed + 40) + coef;
    noise->humidity.n = fsl_perlin_noise_2d(x, y, coef, FREQ_HUMIDITY, world.seed + 50) + coef;
    noise->extremity.n = fsl_perlin_noise_2d(x, y, coef, FREQ_EXTREMITY, world.seed + 60) + coef;
    noise->roughness.n = fsl_perlin_noise_2d(x, y, coef, FREQ_ROUGHNESS, world.seed + 70) + coef;
    noise->life.n = fsl_perlin_noise_2d(x, y, coef, FREQ_LIFE, world.seed + 80) + coef;

    noise->temperature.n = fsl_clamp_f32(noise->temperature.n, 0.0f, 1.0f);
    noise->humidity.n = fsl_clamp_f32(noise->humidity.n, 0.0f, 1.0f);
    noise->extremity.n = fsl_clamp_f32(noise->extremity.n, 0.0f, 1.0f);
    noise->roughness.n = fsl_clamp_f32(noise->roughness.n, 0.0f, 1.0f);
    noise->life.n = fsl_clamp_f32(noise->life.n, 0.0f, 1.0f);

    return CHUNK_WORK_COST_GENERATE_NON_AIR;
}

chunk_work_cost terrain_noise_lerp(hhc_terrain_noise *dst,
        const hhc_terrain_noise *a, const hhc_terrain_noise *b, f32 t)
{
    dst->continental.n += (b->continental.n - a->continental.n) * t;
    dst->regional.n += (b->regional.n - a->regional.n) * t;
    dst->local.n += (b->local.n - a->local.n) * t;
    dst->detail.n += (b->detail.n - a->detail.n) * t;

    dst->temperature.n += (b->temperature.n - a->temperature.n) * t;
    dst->humidity.n += (b->humidity.n - a->humidity.n) * t;
    dst->extremity.n += (b->extremity.n - a->extremity.n) * t;
    dst->roughness.n += (b->roughness.n - a->roughness.n) * t;
    dst->life.n += (b->life.n - a->life.n) * t;

    return CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE;
}

chunk_work_cost terrain_noise_bilerp(hhc_terrain_noise *dst,
        const hhc_terrain_noise *a, const hhc_terrain_noise *b,
        const hhc_terrain_noise *c, const hhc_terrain_noise *d,
        f32 tx, f32 ty)
{
    dst->continental.n = bilerp_f32(a->continental.n, b->continental.n,
            c->continental.n, d->continental.n, tx, ty);
    dst->regional.n = bilerp_f32(a->regional.n, b->regional.n, c->regional.n,
            d->regional.n, tx, ty);
    dst->local.n = bilerp_f32(a->local.n, b->local.n, c->local.n, d->local.n, tx, ty);
    dst->detail.n = bilerp_f32(a->detail.n, b->detail.n, c->detail.n, d->detail.n, tx, ty);

    dst->temperature.n = bilerp_f32(a->temperature.n, b->temperature.n, c->temperature.n,
            d->temperature.n, tx, ty);
    dst->humidity.n = bilerp_f32(a->humidity.n, b->humidity.n, c->humidity.n,
            d->humidity.n, tx, ty);
    dst->extremity.n = bilerp_f32(a->extremity.n, b->extremity.n, c->extremity.n,
            d->extremity.n, tx, ty);
    dst->roughness.n = bilerp_f32(a->roughness.n, b->roughness.n, c->roughness.n,
            d->roughness.n, tx, ty);
    dst->life.n = bilerp_f32(a->life.n, b->life.n, c->life.n, d->life.n, tx, ty);

    return CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE * 2;
}

chunk_work_cost terrain_noise_trilerp(hhc_terrain_noise *dst,
        const hhc_terrain_noise *a, const hhc_terrain_noise *b,
        const hhc_terrain_noise *c, const hhc_terrain_noise *d,
        const hhc_terrain_noise *e, const hhc_terrain_noise *f,
        const hhc_terrain_noise *g, const hhc_terrain_noise *h,
        f32 tx, f32 ty, f32 tz)
{
    dst->continental.n = trilerp_f32(a->continental.n, b->continental.n, c->continental.n,
            d->continental.n, e->continental.n, f->continental.n, g->continental.n,
            h->continental.n, tx, ty, tz);
    dst->regional.n = trilerp_f32(a->regional.n, b->regional.n, c->regional.n,
            d->regional.n, e->regional.n, f->regional.n, g->regional.n, h->regional.n, tx, ty, tz);
    dst->local.n = trilerp_f32(a->local.n, b->local.n, c->local.n, d->local.n, e->local.n,
            f->local.n, g->local.n, h->local.n, tx, ty, tz);
    dst->detail.n = trilerp_f32(a->detail.n, b->detail.n, c->detail.n, d->detail.n, e->detail.n,
            f->detail.n, g->detail.n, h->detail.n, tx, ty, tz);

    dst->temperature.n = trilerp_f32(a->temperature.n, b->temperature.n, c->temperature.n,
            d->temperature.n, e->temperature.n, f->temperature.n, g->temperature.n,
            h->temperature.n, tx, ty, tz);
    dst->humidity.n = trilerp_f32(a->humidity.n, b->humidity.n, c->humidity.n, d->humidity.n,
            e->humidity.n, f->humidity.n, g->humidity.n, h->humidity.n, tx, ty, tz);
    dst->extremity.n = trilerp_f32(a->extremity.n, b->extremity.n, c->extremity.n,
            d->extremity.n, e->extremity.n, f->extremity.n, g->extremity.n, h->extremity.n, tx, ty, tz);
    dst->roughness.n = trilerp_f32(a->roughness.n, b->roughness.n, c->roughness.n,
            d->roughness.n, e->roughness.n, f->roughness.n, g->roughness.n, h->roughness.n, tx, ty, tz);
    dst->life.n = trilerp_f32(a->life.n, b->life.n, c->life.n, d->life.n, e->life.n, f->life.n,
            g->life.n, h->life.n, tx, ty, tz);

    return CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE * 4;
}

hhc_terrain terrain_shape(i32 x, i32 y, i32 z, const hhc_terrain_noise *noise)
{
    hhc_terrain terrain = {0};
    hhc_biome biome = {0};
    f32 biome_score[BIOME_COUNT] = {0};
    f32 biome_best_score = 10000.0f;
    i32 biome_best_index = 0;
    i32 i = BIOME_COUNT;

    biome.temperature = noise->temperature.n;
    biome.humidity = noise->humidity.n;
    biome.extremity = noise->extremity.n;
    biome.roughness = noise->roughness.n;
    biome.life = noise->life.n;

    biome.param[BIOME_PARAM_TEMPERATURE] = noise->temperature.n;
    biome.param[BIOME_PARAM_HUMIDITY] = noise->humidity.n;
    biome.param[BIOME_PARAM_EXTREMITY] = noise->extremity.n;
    biome.param[BIOME_PARAM_ROUGHNESS] = noise->roughness.n;
    biome.param[BIOME_PARAM_LIFE] = noise->life.n;

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
        ((noise->continental.n + noise->regional.n) * (1.0f - noise->temperature.n) +
        noise->local.n) * noise->extremity.n +
        noise->detail.n * noise->roughness.n;

    if ((f32)z < terrain.value)
    {
        terrain.block_id = biome_best_index;
    }

    return terrain;
}
