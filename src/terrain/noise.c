#include "deps/fossil/math/noise.h"

#include "noise.h"

#include <math.h>

/*!
 *  @remark these constants are the result of the mathematical expression "3^174"
 *  split into groups of 6 digits, I chose this power because it was the first
 *  power of 3 that included exactly the number of digits to fit into 14 6-digit
 *  long constants with none of them starting with a zero.
 */
#define RAND_CONST_0 104495
#define RAND_CONST_1 676331
#define RAND_CONST_2 778315
#define RAND_CONST_3 966103
#define RAND_CONST_4 878903
#define RAND_CONST_5 450701
#define RAND_CONST_6 989608
#define RAND_CONST_7 781073
#define RAND_CONST_8 244439
#define RAND_CONST_9 950619
#define RAND_CONST_10 431748
#define RAND_CONST_11 912396
#define RAND_CONST_12 904023
#define RAND_CONST_13 371769

f32 bilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 tx, f32 ty)
{
    f32 wx = 0.0f;
    f32 wy = 0.0f;
    f32 w[4] = {0};

    wx = 1.0f - tx;
    wy = 1.0f - ty;
    w[0] = wx * wy;
    w[1] = tx * wy;
    w[2] = wx * ty;
    w[3] = tx * ty;

    return
        a * w[0] +
        b * w[1] +
        c * w[2] +
        d * w[3];
}

f32 trilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h, f32 tx, f32 ty, f32 tz)
{
    f32 wx = 0.0f;
    f32 wy = 0.0f;
    f32 wz = 0.0f;
    f32 w[8] = {0};

    wx = 1.0f - tx;
    wy = 1.0f - ty;
    wz = 1.0f - tz;
    w[0] = wx * wy * wz;
    w[1] = tx * wy * wz;
    w[2] = wx * ty * wz;
    w[3] = tx * ty * wz;
    w[4] = wx * wy * tz;
    w[5] = tx * wy * tz;
    w[6] = wx * ty * tz;
    w[7] = tx * ty * tz;

    return
        a * w[0] +
        b * w[1] +
        c * w[2] +
        d * w[3] +
        e * w[4] +
        f * w[5] +
        g * w[6] +
        h * w[7];
}

v2f32 gradient_2d(i32 x, i32 y, u64 seed)
{
    v2f32 v = {0};
    u64 h = {0};
    h = x * RAND_CONST_1;
    h ^= y * RAND_CONST_2;
    h ^= h >> 16;
    h *= RAND_CONST_0;

    v.x = fsl_rand_tab[(seed + ((h >> 0) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    v.y = fsl_rand_tab[(seed + ((h >> 30) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    return v;
}

v3f32 gradient_3d(i32 x, i32 y, i32 z, u64 seed)
{
    v3f32 v = {0};
    u64 h = {0};
    h = x * RAND_CONST_1;
    h ^= y * RAND_CONST_2;
    h ^= z * RAND_CONST_3;
    h ^= h >> 16;
    h *= RAND_CONST_0;
    v.x = fsl_rand_tab[(seed + ((h >> 0) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    v.y = fsl_rand_tab[(seed + ((h >> 20) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    v.z = fsl_rand_tab[(seed + ((h >> 40) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    return v;
}

chunk_work_cost noise_sample_axis_init(hhc_noise_sample *s, u32 axis, f32 x, f32 frequency)
{
    f32 v = x * frequency;
    i32 a = (i32)floorf(v);
    i32 b = a + 1;
    f32 d = v - (f32)a;
    d = d * d * d * (d * (d * 6.0f - 15.0f) + 10.0f);

    s->v[axis] = v;
    s->a[axis] = a;
    s->b[axis] = b;
    s->dv[axis] = d;
    s->da[axis] = v - (f32)a;
    s->db[axis] = v - (f32)b;
    return CHUNK_WORK_COST_GENERATE_NOISE_INIT;
}

chunk_work_cost noise_sample_make_2d(hhc_noise_sample *s, f32 amplitude, u64 seed)
{
    f32 dx = s->dv[0];
    f32 dy = s->dv[1];
    f32 wx = 1.0f - s->dv[0];
    f32 wy = 1.0f - s->dv[1];
    f32 n[4] = {0};
    v2f32 g[4] = {0};

    g[0] = gradient_2d(s->a[0], s->a[1], seed);
    g[1] = gradient_2d(s->b[0], s->a[1], seed);
    g[2] = gradient_2d(s->a[0], s->b[1], seed);
    g[3] = gradient_2d(s->b[0], s->b[1], seed);

    n[0] = s->da[0] * g[0].x + s->da[1] * g[0].y;
    n[1] = s->db[0] * g[1].x + s->da[1] * g[1].y;
    n[2] = s->da[0] * g[2].x + s->db[1] * g[2].y;
    n[3] = s->db[0] * g[3].x + s->db[1] * g[3].y;

    s->n =
        (n[0] * wx * wy +
         n[1] * dx * wy +
         n[2] * wx * dy +
         n[3] * dx * dy) * amplitude;
    return CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE;
}

chunk_work_cost noise_sample_make_3d(hhc_noise_sample *s, f32 amplitude, u64 seed)
{
    f32 dx = s->dv[0];
    f32 dy = s->dv[1];
    f32 dz = s->dv[2];
    f32 wx = 1.0f - s->dv[0];
    f32 wy = 1.0f - s->dv[1];
    f32 wz = 1.0f - s->dv[2];
    f32 n[8] = {0};
    v3f32 g[8] = {0};

    g[0] = gradient_3d(s->a[0], s->a[1], s->a[2], seed);
    g[1] = gradient_3d(s->b[0], s->a[1], s->a[2], seed);
    g[2] = gradient_3d(s->a[0], s->b[1], s->a[2], seed);
    g[3] = gradient_3d(s->b[0], s->b[1], s->a[2], seed);
    g[4] = gradient_3d(s->a[0], s->a[1], s->b[2], seed);
    g[5] = gradient_3d(s->b[0], s->a[1], s->b[2], seed);
    g[6] = gradient_3d(s->a[0], s->b[1], s->b[2], seed);
    g[7] = gradient_3d(s->b[0], s->b[1], s->b[2], seed);

    n[0] = s->da[0] * g[0].x + s->da[1] * g[0].y + s->da[2] * g[0].z;
    n[1] = s->db[0] * g[1].x + s->da[1] * g[1].y + s->da[2] * g[1].z;
    n[2] = s->da[0] * g[2].x + s->db[1] * g[2].y + s->da[2] * g[2].z;
    n[3] = s->db[0] * g[3].x + s->db[1] * g[3].y + s->da[2] * g[3].z;
    n[4] = s->da[0] * g[4].x + s->da[1] * g[4].y + s->db[2] * g[4].z;
    n[5] = s->db[0] * g[5].x + s->da[1] * g[5].y + s->db[2] * g[5].z;
    n[6] = s->da[0] * g[6].x + s->db[1] * g[6].y + s->db[2] * g[6].z;
    n[7] = s->db[0] * g[7].x + s->db[1] * g[7].y + s->db[2] * g[7].z;

    s->n =
        (n[0] * wx * wy * wz +
         n[1] * dx * wy * wz +
         n[2] * wx * dy * wz +
         n[3] * dx * dy * wz +
         n[4] * wx * wy * dz +
         n[5] * dx * wy * dz +
         n[6] * wx * dy * dz +
         n[7] * dx * dy * dz) * amplitude;
    return CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE;
}
