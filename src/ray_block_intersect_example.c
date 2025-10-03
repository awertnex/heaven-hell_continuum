#include "engine/h/defines.h"
#include "engine/h/math.h"

typedef struct BlockHitInfo
{
    v3i32 block_pos;
    v3i8 face_normal;
    f32 distance;
    b8 hit;
} BlockHitInfo;

#define MAX_REACH_DISTANCE 5.0f
u8 block_buf[16][16][16] = {1};

/* ---- signatures ---------------------------------------------------------- */
u64 factorial(i32 n);
f64 pow_f64(f64 n, u8 exp);
f64 sine_f64(f64 x);

b8
raycast_block_hit(v3f32 camera, v3f32 target,
        u8 target_type, BlockHitInfo *out_hit)
{
    v3f32 dir = normalize_v3f32(sub_v3f32(target, camera));
    i32 x = (i32)floorf(camera.x);
    i32 y = (i32)floorf(camera.y);
    i32 z = (i32)floorf(camera.z);

    i32 step_x = (dir.x > 0) ? 1 : -1;
    i32 step_y = (dir.y > 0) ? 1 : -1;
    i32 step_z = (dir.z > 0) ? 1 : -1;

    float t_max_x =
        (floorf(camera.x) + (step_x > 0 ? 1.0f : 0.0f) - camera.x) / dir.x;

    float t_max_y =
        (floorf(camera.y) + (step_y > 0 ? 1.0f : 0.0f) - camera.y) / dir.y;

    float t_max_z =
        (floorf(camera.z) + (step_z > 0 ? 1.0f : 0.0f) - camera.z) / dir.z;

    float t_delta_x = fabsf(1.0f / dir.x);
    float t_delay_y = fabsf(1.0f / dir.y);
    float t_delta_z = fabsf(1.0f / dir.z);

    int last_axis = -1;

    for (;;)
    {
        if (x < 0 || x >= 64 || y < 0 || y >= 64 || z < 0 || z >= 420)
            break;

        if (block_buf[z][y][x] == target_type)
        {
            v3f32 center = (v3f32){x + 0.5f, y + 0.5f, z + 0.5f};
            f32 dist = distance_v3f32(camera, center);

            if (dist <= MAX_REACH_DISTANCE)
            {
                /* Fill out hit info */
                out_hit->block_pos = (v3i32){x, y, z};
                out_hit->distance = dist;
                out_hit->hit = TRUE;

                /* Determine face normal */
                v3i8 normal = {0};
                switch (last_axis)
                {
                    case 0: normal.x = -step_x; break;
                    case 1: normal.y = -step_y; break;
                    case 2: normal.z = -step_z; break;
                }

                out_hit->face_normal = normal;
                return TRUE;
            }
        }

        /* Step to next voxel */
        if (t_max_x < t_max_y)
        {
            if (t_max_x < t_max_z)
            {
                x += step_x;
                t_max_x += t_delta_x;
                last_axis = 0;
            }
            else
            {
                z += step_z;
                t_max_z += t_delta_z;
                last_axis = 2;
            }
        }
        else
        {
            if (t_max_y < t_max_z)
            {
                y += step_y;
                t_max_y += t_delay_y;
                last_axis = 1;
            }
            else
            {
                z += step_z;
                t_max_z += t_delta_z;
                last_axis = 2;
            }
        }

        /* Optional: early exit */
        v3f32 blockCenter = (v3f32){x + 0.5f, y + 0.5f, z + 0.5f};
        if (distance_v3f32(camera, blockCenter) > MAX_REACH_DISTANCE)
            break;
    }

    out_hit->hit = false;
    return false;
}

u64
factorial(i32 n)
{
    if (n == 0) return 1;
    u64 result = 1;
    for (i32 i = 2; i <= n; ++i)
        result *= i;
    return result;
}

f64
pow_f64(f64 n, u8 exp)
{
    f64 result = 1.0;
    for (u8 i = 0; i < exp; ++i)
        result *= n;
    return result;
}

/* Approximate sine using Taylor Series */
f64
sin_f64(f64 x)
{
    while (x > PI)
        x -= 2 * PI;
    while (x < -PI)
        x += 2 * PI;

    f64 result = 0.0f;
    u8 precision = 10;
    for (u8 n = 0; n < precision; ++n)
    {
        f64 term = pow_f64(-1, n) * pow_f64(x, 2*n + 1) / factorial(2*n + 1);
        result += term;
    }
    return result;
}
