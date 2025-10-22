#version 430 core

#define VIGNETTE 0.2
#define GRAIN_INTENSITY 0.001
#define GRAIN_SEED 5000.0
#define GRAIN_SIZE 60.0

uniform sampler2D texture_screen;
uniform float time;
in vec2 vs_pos;
in vec2 tex_coords;
out vec4 color;

float rand(float seed)
{
    uint w = 8 * 8;
    uint s = w / 2;
    uint a = w, b = s + uint(seed * 1000.0);
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;
    return (sin(a * (3.14159 / ~(~0u >> 1))) + 1.0) / 2.0;
}

void main()
{ 
    float vignette = pow(length(vs_pos) * VIGNETTE, 3.0);
    vignette = clamp(vignette, 0.0, 1.0);

    float grain = rand(time + (vs_pos.y / GRAIN_SIZE) + GRAIN_SEED);

    color = texture(texture_screen, tex_coords);
    color.rgb -= vignette;
    color.rgb -= (grain * GRAIN_INTENSITY);
}
