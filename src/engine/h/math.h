#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#include <math.h>

#include "defines.h"

/* ---- section: definitions ------------------------------------------------ */

#define PI          3.14159265358979323846
#define DEG2RAD     (PI / 180.0f)   /* 0.017453293f */
#define RAD2DEG     (180.0f / PI)   /* 57.295779513f */

#define GRAVITY     (9.7803267715f / 100.0f)

#define v3izero     (v3i32){0, 0, 0}
#define v3fzero     (v3f32){0.0e-5f, 0.0e-5f, 0.0e-5f}

/* ---- section: signatures ------------------------------------------------- */

v3f32 add_v3f32(v3f32 a, v3f32 b);
v3f32 sub_v3f32(v3f32 a, v3f32 b);
f32 clamp_f32(f32 val, f32 min, f32 max);
f64 clamp_f64(f64 val, f64 min, f64 max);
f32 len_v3f32(v3f32 v);
v3f32 normalize_v3f32(v3f32 v);
u32 distance_v3i32(v3i32 a, v3i32 b);
f32 distance_v3f32(v3f32 a, v3f32 b);
b8 is_in_range_i32(i32 pos, i32 start, i32 end);
b8 is_in_range_f32(f32 pos, f32 start, f32 end);
b8 is_in_area_f32(v2f32 pos, v2f32 start, v2f32 end);
b8 is_in_volume_i32(v3i32 pos, v3i32 start, v3i32 end);
b8 is_in_volume_f32_i32(v3f32 pos, v3i32 start, v3i32 end);

m4f32 matrix_add(m4f32 a, m4f32 b);
m4f32 matrix_subtract(m4f32 a, m4f32 b);
m4f32 matrix_multiply(m4f32 a, m4f32 b);
v4f32 matrix_multiply_vector(m4f32 a, v4f32 b);

#endif /* ENGINE_MATH_H */

