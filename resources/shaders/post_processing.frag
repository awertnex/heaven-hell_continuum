#version 430 core

#define VIGNETTE_NARROWNESS 0.6
#define VIGNETTE_INTENSITY 0.2
#define GRAIN_INTENSITY 0.1

uniform sampler2D texture_screen;
uniform uint time;
in vec2 vs_pos;
in vec2 tex_coords;
out vec4 color;

void main()
{ 
    float vignette = pow(length(vs_pos) * VIGNETTE_NARROWNESS, 3.0);
    vignette = clamp(vignette, 0.0, 1.0) * VIGNETTE_INTENSITY;

    float grain = dot(vs_pos * ((time % 5000) + 1), vec2(-vs_pos.y, 78.233));
    grain = sin(grain) * 43758.5453;
    grain = ((fract(grain) * 2.0) - 1.0) * GRAIN_INTENSITY;

    color = texture(texture_screen, tex_coords);
    color.rgb *= (1.0 + grain);
    color.rgb -= vignette;
    color.rgb = mix(color.rgb, color.rgb * 2.0, vignette);
}
