#include "h/math.h"

v3f32
add_v3f32(v3f32 a, v3f32 b)
{
    return (v3f32){a.x + b.x, a.y + b.y, a.z + b.z};
}

v3f32
sub_v3f32(v3f32 a, v3f32 b)
{
    return (v3f32){a.x - b.x, a.y - b.y, a.z - b.z};
}

i32
clamp_i32(i32 val, i32 min, i32 max)
{
    return ((val < min) ? min : ((val > max) ? max : val));
}

u32
clamp_u32(u32 val, u32 min, u32 max)
{
    return ((val < min) ? min : ((val > max) ? max : val));
}

f32
clamp_f32(f32 val, f32 min, f32 max)
{
    return ((val < min) ? min : ((val > max) ? max : val));
}

i64
clamp_i64(i64 val, i64 min, i64 max)
{
    return ((val < min) ? min : ((val > max) ? max : val));
}

u64
clamp_u64(u64 val, u64 min, u64 max)
{
    return ((val < min) ? min : ((val > max) ? max : val));
}

f64
clamp_f64(f64 val, f64 min, f64 max)
{
    return ((val < min) ? min : ((val > max) ? max : val));
}

f32
len_v3f32(v3f32 v)
{
    return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

v3f32
normalize_v3f32(v3f32 v)
{
    f32 len = len_v3f32(v);
    if (len == 0)
        return v3fzero;

    return (v3f32){v.x / len, v.y / len, v.z / len};
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
f32
q_rsqrt(f32 n)
{
    i64 i;
    f32 x2, y;
    const f32 threehalfs = 1.5f;

    x2 = n * 0.5f;
    y = n;
    i = *(i64*)&y;                          /* evil floating point bit hack */
    i = 0x5f3759df - (i >> 1);              /* what the fuck? */
    y = *(f32*)&i;
    y = (y * (threehalfs - (x2 * y * y)));  /* 1st iteration */
    y = (y * (threehalfs - (x2 * y * y)));  /* 2nd iteration, can be removed */

    return y;
}
#pragma GCC diagnostic pop

u32
distance_v3i32(v3i32 a, v3i32 b)
{
    return (u32)
        (powf(a.x - b.x, 2.0f) +
         powf(a.y - b.y, 2.0f) +
         powf(a.z - b.z, 2.0f));
}

f32
distance_v3f32(v3f32 a, v3f32 b)
{
    return
        powf(a.x - b.x, 2.0f) +
        powf(a.y - b.y, 2.0f) +
        powf(a.z - b.z, 2.0f);
}

b8
is_in_range_i32(i32 pos, i32 start, i32 end)
{
    return ((pos - start) >= 0) & ((end - pos) >= 0);
}

b8
is_in_range_f32(f32 pos, f32 start, f32 end)
{
    return ((pos - start) >= 0.0f) & ((end - pos) >= 0.0f);
}

b8
is_in_range_i64(i64 pos, i64 start, i64 end)
{
    return ((pos - start) >= 0) & ((end - pos) >= 0);
}

b8
is_in_range_f64(f64 pos, f64 start, f64 end)
{
    return ((pos - start) >= 0.0f) & ((end - pos) >= 0.0f);
}

b8
is_in_area_i32(v2i32 pos, v2i32 start, v2i32 end)
{
    return
        ((pos.x - start.x) >= 0) & ((end.x - pos.x) >= 0) &
        ((pos.y - start.y) >= 0) & ((end.y - pos.y) >= 0);
}

b8
is_in_area_f32(v2f32 pos, v2f32 start, v2f32 end)
{
    return
        ((pos.x - start.x) >= 0.0f) & ((end.x - pos.x) >= 0.0f) &
        ((pos.y - start.y) >= 0.0f) & ((end.y - pos.y) >= 0.0f);
}

b8
is_in_volume_i32(v3i32 pos, v3i32 start, v3i32 end)
{
    return
        ((pos.x - start.x) >= 0) & ((end.x - pos.x) >= 0) &
        ((pos.y - start.y) >= 0) & ((end.y - pos.y) >= 0) &
        ((pos.z - start.z) >= 0) & ((end.z - pos.z) >= 0);
}

b8
is_in_volume_f32(v3f32 pos, v3f32 start, v3f32 end)
{
    return
        ((pos.x - start.x) >= 0.0f) & ((end.x - pos.x) >= 0.0f) &
        ((pos.y - start.y) >= 0.0f) & ((end.y - pos.y) >= 0.0f) &
        ((pos.z - start.z) >= 0.0f) & ((end.z - pos.z) >= 0.0f);
}

b8
is_in_volume_i64(v3i64 pos, v3i64 start, v3i64 end)
{
    return
        ((pos.x - start.x) >= 0) & ((end.x - pos.x) >= 0) &
        ((pos.y - start.y) >= 0) & ((end.y - pos.y) >= 0) &
        ((pos.z - start.z) >= 0) & ((end.z - pos.z) >= 0);
}

m4f32
matrix_add(m4f32 a, m4f32 b)
{
    return (m4f32){
        ((a.a11) + (b.a11)), ((a.a12) + (b.a12)),
            ((a.a13) + (b.a13)), ((a.a14) + (b.a14)),

            ((a.a21) + (b.a21)), ((a.a22) + (b.a22)),
            ((a.a23) + (b.a23)), ((a.a24) + (b.a24)),

            ((a.a31) + (b.a31)), ((a.a32) + (b.a32)),
            ((a.a33) + (b.a33)), ((a.a34) + (b.a34)),

            ((a.a41) + (b.a41)), ((a.a42) + (b.a42)),
            ((a.a43) + (b.a43)), ((a.a44) + (b.a44)),
    };
}

m4f32
matrix_subtract(m4f32 a, m4f32 b)
{
    return (m4f32){
        ((a.a11) - (b.a11)), ((a.a12) - (b.a12)),
            ((a.a13) - (b.a13)), ((a.a14) - (b.a14)),

            ((a.a21) - (b.a21)), ((a.a22) - (b.a22)),
            ((a.a23) - (b.a23)), ((a.a24) - (b.a24)),

            ((a.a31) - (b.a31)), ((a.a32) - (b.a32)),
            ((a.a33) - (b.a33)), ((a.a34) - (b.a34)),

            ((a.a41) - (b.a41)), ((a.a42) - (b.a42)),
            ((a.a43) - (b.a43)), ((a.a44) - (b.a44)),
    };
}

m4f32
matrix_multiply(m4f32 a, m4f32 b)
{
    return (m4f32){
        ((a.a11) * (b.a11)) + ((a.a12) * (b.a21)) +
            ((a.a13) * (b.a31)) + ((a.a14) * (b.a41)),

            ((a.a11) * (b.a12)) + ((a.a12) * (b.a22)) +
                ((a.a13) * (b.a32)) + ((a.a14) * (b.a42)),

            ((a.a11) * (b.a13)) + ((a.a12) * (b.a23)) +
                ((a.a13) * (b.a33)) + ((a.a14) * (b.a43)),

            ((a.a11) * (b.a14)) + ((a.a12) * (b.a24)) +
                ((a.a13) * (b.a34)) + ((a.a14) * (b.a44)),


            ((a.a21) * (b.a11)) + ((a.a22) * (b.a21)) +
                ((a.a23) * (b.a31)) + ((a.a24) * (b.a41)),

            ((a.a21) * (b.a12)) + ((a.a22) * (b.a22)) +
                ((a.a23) * (b.a32)) + ((a.a24) * (b.a42)),

            ((a.a21) * (b.a13)) + ((a.a22) * (b.a23)) +
                ((a.a23) * (b.a33)) + ((a.a24) * (b.a43)),

            ((a.a21) * (b.a14)) + ((a.a22) * (b.a24)) +
                ((a.a23) * (b.a34)) + ((a.a24) * (b.a44)),


            ((a.a31) * (b.a11)) + ((a.a32) * (b.a21)) +
                ((a.a33) * (b.a31)) + ((a.a34) * (b.a41)),

            ((a.a31) * (b.a12)) + ((a.a32) * (b.a22)) +
                ((a.a33) * (b.a32)) + ((a.a34) * (b.a42)),

            ((a.a31) * (b.a13)) + ((a.a32) * (b.a23)) +
                ((a.a33) * (b.a33)) + ((a.a34) * (b.a43)),

            ((a.a31) * (b.a14)) + ((a.a32) * (b.a24)) +
                ((a.a33) * (b.a34)) + ((a.a34) * (b.a44)),


            ((a.a41) * (b.a11)) + ((a.a42) * (b.a21)) +
                ((a.a43) * (b.a31)) + ((a.a44) * (b.a41)),

            ((a.a41) * (b.a12)) + ((a.a42) * (b.a22)) +
                ((a.a43) * (b.a32)) + ((a.a44) * (b.a42)),

            ((a.a41) * (b.a13)) + ((a.a42) * (b.a23)) +
                ((a.a43) * (b.a33)) + ((a.a44) * (b.a43)),

            ((a.a41) * (b.a14)) + ((a.a42) * (b.a24)) +
                ((a.a43) * (b.a34)) + ((a.a44) * (b.a44)),
    };
}

v4f32
matrix_multiply_vector(m4f32 a, v4f32 b)
{
    return (v4f32){
        ((a.a11) * (b.x)) + ((a.a12) * (b.x)) +
            ((a.a13) * (b.x)) + ((a.a14) * (b.x)),

            ((a.a21) * (b.y)) + ((a.a22) * (b.y)) +
                ((a.a23) * (b.y)) + ((a.a24) * (b.y)),

            ((a.a31) * (b.z)) + ((a.a32) * (b.z)) +
                ((a.a33) * (b.z)) + ((a.a34) * (b.z)),

            ((a.a41) * (b.w)) + ((a.a42) * (b.w)) +
                ((a.a43) * (b.w)) + ((a.a44) * (b.w)),
    };
}

f32
lerp_f32(f32 start, f32 end, f32 speed, f64 frame_delta)
{
    return start + (end - start) * (1.0f - expf(-speed * frame_delta));
}

f32
easein_f32(f32 start, f32 end, f32 scale)
{
    return start + (end - start) * (scale * scale);
}

f32 smoothstep(f32 a, f32 b, f32 i)
{
   i = clamp_f32((i - a) / (b - a), -1.0f, 1.0f);
   return i * i * (3.0f - 2.0f * i);
}
