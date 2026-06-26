#ifndef HHC_TERRAIN_NOISE_H
#define HHC_TERRAIN_NOISE_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"

#include "../chunking/chunk_work.h"

typedef struct hhc_noise_sample
{
    f32 v[3];   /* position */
    i32 a[3];   /* `v` floor */
    i32 b[3];   /* `v` ceil */
    f32 dv[3];  /* `v` delta */
    f32 da[3];  /* `a` delta */
    f32 db[3];  /* `b` delta */
    f32 n;      /* final value */
} hhc_noise_sample;

f32 bilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 tx, f32 ty);
f32 trilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h, f32 tx, f32 ty, f32 tz);
v2f32 gradient_2d(i32 x, i32 y, u64 seed);
v3f32 gradient_3d(i32 x, i32 y, i32 z, u64 seed);
chunk_work_cost noise_sample_axis_init(hhc_noise_sample *s, u32 axis, f32 x, f32 frequency);
chunk_work_cost noise_sample_make_2d(hhc_noise_sample *s, f32 amplitude, u64 seed);
chunk_work_cost noise_sample_make_3d(hhc_noise_sample *s, f32 amplitude, u64 seed);

#endif /* HHC_TERRAIN_NOISE_H */
