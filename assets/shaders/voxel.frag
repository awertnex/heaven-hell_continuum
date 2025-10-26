#version 430 core

uniform sampler2D texture_block;
uniform vec3 player_position;
uniform float opacity;
in vec3 position;
in vec2 tex_coords;
in vec3 normal;
out vec4 color;

#include "h/defaults.frag.h"

vec3 base_color = vec3(1.0, 0.74, 0.41);
vec4 base_texture = texture(texture_block, tex_coords);
float distance = length(position - player_position);
float flashlight = FLASHLIGHT_INTENSITY / (distance * FALLOFF);

void main()
{
    base_color *= base_texture.rgb;
    base_color += (sun_direction * SUN_INFLUENCE);
    base_color *= sky_brightness + flashlight;
    base_color = mix(base_color.rgb, sky_color.rgb, sky_color_influence);

    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    /* brightness & contrast */
    base_color *= BRIGHTNESS;
    base_color = ((base_color - CONTRAST) * (CONTRAST + 1.0)) + CONTRAST;
    color = vec4(base_color, 1.0) * base_texture * opacity;
}
