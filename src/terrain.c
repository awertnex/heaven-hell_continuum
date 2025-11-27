#include <engine/h/math.h>

#include "h/main.h"
#include "h/terrain.h"

static v3f32 random(f32 x0, f32 y0, u32 seed);
static f32 gradient(f32 x0, f32 y0, f32 x, f32 y);
static f32 interp(f32 a, f32 b, f32 scale);

static v3f32 random(f32 x0, f32 y0, u32 seed)
{
    const u32 w = 8 * sizeof(u64);
    const u32 s = w / 2; 
    u32 a = (i32)x0 + seed + 5000, b = (i32)y0 - seed - 5000;
    a *= 3284157443;
 
    b ^= a << s | a >> (w - s);
    b *= 1911520717;
 
    a ^= b << s | b >> (w - s);
    a *= 2048419325;
    f32 final = a * (PI / ~(~0u >> 1));
    
    return (v3f32){
        sin(final),
        cos(final),
        1.0f,
    };
}

static f32 gradient(f32 x0, f32 y0, f32 x, f32 y)
{
    v3f32 grad = random(x0, y0, SET_TERRAIN_SEED_DEFAULT);
    f32 dx = x0 - x;
    f32 dy = y0 - y;
    return dx * grad.x + dy * grad.y;
}

static f32 interp(f32 a, f32 b, f32 t)
{
    return (b - a) * (3.0f - t * 2.0f) * t * t + a;
}

f32 terrain_noise(v3i32 coordinates, f32 amplitude, f32 frequency)
{
    f32 x = (f32)coordinates.x / frequency;
    f32 y = (f32)coordinates.y / frequency;
    i32 x0 = (i32)floorf(x);
    i32 y0 = (i32)floorf(y);
    i32 x1 = x0 + 1;
    i32 y1 = y0 + 1;

    f32 sx = x - (f32)x0;
    f32 sy = y - (f32)y0;

    f32 n0 = gradient(x0, y0, x, y);
    f32 n1 = gradient(x1, y0, x, y);
    f32 ix0 = interp(n0, n1, sx);

    n0 = gradient(x0, y1, x, y);
    n1 = gradient(x1, y1, x, y);
    f32 ix1 = interp(n0, n1, sx);

    return interp(ix0, ix1, sy) * amplitude;
}

f32 terrain_land(v3i32 coordinates)
{
    f32 elevation = clamp_f32(
            terrain_noise(coordinates, 1.0f, 329.0f) + 0.5f, 0.0f, 1.0f);
    f32 influence = terrain_noise(coordinates, 1.0, 50.0f);
    f32 terrain = terrain_noise(coordinates, 250.0f, 256.0f) * elevation;
    terrain += terrain_noise(coordinates, 30.0f, 40.0f) * elevation;
    terrain += (terrain_noise(coordinates, 10.0f, 10.0f) * influence);
    terrain += expf(-terrain_noise(coordinates, 8.0f, 150.0f));

    return terrain;
}
