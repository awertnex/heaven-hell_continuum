#include <engine/h/math.h>
#include <engine/h/memory.h>

#include "h/dir.h"
#include "h/main.h"
#include "h/terrain.h"

/* random number look-up table */
f32 *RAND_TAB = {0};

u32 rand_init(void)
{
    str file_name[PATH_MAX] = {0};
    f32 *file_contents = NULL;
    u64 file_len = 0;
    i32 i;

    if (mem_map((void*)&RAND_TAB, SET_RAND_TAB_MAX * sizeof(f32),
                "terrain_init().RAND_TAB") != ERR_SUCCESS)
        goto cleanup;

    snprintf(file_name, PATH_MAX, "%slookup_rand_tab.bin", DIR_ROOT[DIR_LOOKUPS]);

    if (is_file_exists(file_name, FALSE) == ERR_SUCCESS)
    {
        file_len = get_file_contents(file_name,
                (void*)&file_contents, sizeof(f32), "rb", FALSE);
        if (*GAME_ERR != ERR_SUCCESS || file_contents == NULL)
            goto cleanup;

        for (i = 0; i < SET_RAND_TAB_MAX; ++i)
            RAND_TAB[i] = file_contents[i];

        mem_free((void*)&file_contents, file_len,
                "terrain_init().file_contents");
    }
    else
    {

        for (i = 0; i < SET_RAND_TAB_MAX; ++i)
            RAND_TAB[i] = rand_f32(i);

        if (write_file(file_name, sizeof(i32), SET_RAND_TAB_MAX,
                    RAND_TAB, "wb", TRUE) != ERR_SUCCESS)
            goto cleanup;
    }

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;

cleanup:
    rand_free();
    return *GAME_ERR;
}

void rand_free(void)
{
    mem_unmap((void*)&RAND_TAB, SET_RAND_TAB_MAX * sizeof(f32),
            "terrain_free().RAND_TAB");
}

v3f32 random_2d(i32 x, i32 y, u32 seed)
{
    const u32 S = 32;
    u32 a = x + seed + 94580993;
    u32 b = y - seed - 35786975;

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    a *= 2048419325;
    f32 final = a * RAND_SCALE;

    return (v3f32){
        sin(final),
        cos(final),
        sin(final + 0.25f),
    };
}

v3f32 random_3d(i32 x, i32 y, i32 z, u32 seed)
{
    const u32 S = 32;
    u32 a = x + seed + 7467244;
    u32 b = y - seed - 4909393;
    u32 c = z + seed + 2500462;

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    a *= 2048419325;
    c ^= a << S | b >> S;
    c *= 3567382653;
    f32 final = c * RAND_SCALE;

    return (v3f32){
        sin(final),
        cos(final),
        sin(final + 0.25f),
    };
}

f32 gradient_2d(f32 vx, f32 vy, f32 ax, f32 ay)
{
    f32 sample = RAND_TAB[SET_TERRAIN_SEED_DEFAULT +
        (u32)(734 + ax * 87654 + ay) % SET_RAND_TAB_MAX];
    return
        (vx - ax) * sample +
        (vy - ay) * sample;
}

f32 gradient_3d(f32 vx, f32 vy, f32 vz, f32 ax, f32 ay, f32 az)
{
    v3f32 grad = random_3d(ax, ay, az, SET_TERRAIN_SEED_DEFAULT);
    return
        (ax - vx) * grad.x +
        (ay - vy) * grad.y +
        (az - vz) * grad.z;
}

f32 terrain_noise_2d(v3i32 coordinates, f32 amplitude, f32 frequency)
{
    f32 vx = (f32)coordinates.x / frequency;
    f32 vy = (f32)coordinates.y / frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 bx = ax + 1;
    i32 by = ay + 1;

    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;

    f32 g0 = gradient_2d(vx, vy, ax, ay);
    f32 g1 = gradient_2d(vx, vy, bx, ay);
    f32 l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_2d(vx, vy, ax, by);
    g1 = gradient_2d(vx, vy, bx, by);
    f32 l1 = lerp_cubic_f32(g0, g1, dx);

    return lerp_cubic_f32(l0, l1, dy) * amplitude;
}

f32 terrain_noise_3d(v3i32 coordinates, f32 amplitude, f32 frequency)
{
    f32 vx = (f32)coordinates.x / frequency;
    f32 vy = (f32)coordinates.y / frequency;
    f32 vz = (f32)coordinates.z / frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 az = (i32)floorf(vz);
    i32 bx = ax + 1;
    i32 by = ay + 1;
    i32 bz = az + 1;
    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;
    f32 dz = vz - (f32)az;

    f32 g0 = gradient_3d(ax, ay, az, vx, vy, vz);
    f32 g1 = gradient_3d(bx, ay, az, vx, vy, vz);
    f32 l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_3d(ax, by, az, vx, vy, vz);
    g1 = gradient_3d(bx, by, az, vx, vy, vz);
    f32 l1 = lerp_cubic_f32(g0, g1, dx);

    f32 l2 = lerp_cubic_f32(l0, l1, dy);

    return l2 * amplitude;
}

b8 terrain_land(v3i32 coordinates)
{
    f32 elevation = clamp_f32(
            terrain_noise_2d(coordinates, 1.0f, 329.0f) + 0.5f, 0.0f, 1.0f);
    f32 influence = terrain_noise_2d(coordinates, 1.0, 50.0f);
    f32 terrain = terrain_noise_2d(coordinates, 250.0f, 256.0f) * elevation;
    terrain += terrain_noise_2d(coordinates, 30.0f, 40.0f) * elevation;
    terrain += (terrain_noise_2d(coordinates, 10.0f, 10.0f) * influence);
    terrain += expf(-terrain_noise_2d(coordinates, 8.0f, 150.0f));

    return terrain > coordinates.z;
}
