#ifndef MC_C_MATH_H

#include <math.h>

#define VECTOR4_TYPES
#define MATRIX4_TYPES
#include "defines.h"

// ---- definitions ------------------------------------------------------------
#define PI          3.14159265358979323846f
#define DEG2RAD     (PI / 180.0f)   // 0.017453293f
#define RAD2DEG     (180.0f / PI)   // 0.017453293f

#define arr_len(arr)    (sizeof(arr) / sizeof(arr[0]))

// ---- signatures -------------------------------------------------------------
m4f32 matrix_add(m4f32 a, m4f32 b);
m4f32 matrix_subtract(m4f32 a, m4f32 b);
m4f32 matrix_multiply(m4f32 a, m4f32 b);
v4f32 matrix_multiply_vector(m4f32 a, v4f32 b);

#define MC_C_MATH_H
#endif

