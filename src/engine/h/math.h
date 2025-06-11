#ifndef MC_C_ENGINE_MATH_H
#define MC_C_ENGINE_MATH_H

#include <math.h>

#include "defines.h"

/* ---- section: definitions ------------------------------------------------ */

#ifndef PI
    #define PI      3.14159265358979323846f
#endif /* PI */
#define DEG2RAD     (PI / 180.0f)   /* 0.017453293f */
#define RAD2DEG     (180.0f / PI)   /* 57.295779513f */

#define GRAVITY     (9.7803267715f / 100.0f)

#define v3izero     ((v3i32){0, 0, 0})
#define v3fzero     ((v3f32){0.0e-5f, 0.0e-5f, 0.0e-5f})

/* ---- section: signatures ------------------------------------------------- */

f32 clamp_f32(f32 val, f32 min, f32 max);
f64 clamp_f64(f64 val, f64 min, f64 max);

m4f32 matrix_add(m4f32 a, m4f32 b);
m4f32 matrix_subtract(m4f32 a, m4f32 b);
m4f32 matrix_multiply(m4f32 a, m4f32 b);
v4f32 matrix_multiply_vector(m4f32 a, v4f32 b);

#endif /* MC_C_ENGINE_MATH_H */

