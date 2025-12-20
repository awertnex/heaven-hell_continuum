#version 430 core

#extension GL_ARB_bindless_texture: require

layout(binding = 1, std430) readonly buffer ssbo_textures
{
    sampler2D textures[];
};

uniform sampler2D texture_block;
uniform vec3 player_position;
uniform float opacity;
uniform float toggle_flashlight;
in vec3 position;
in vec2 tex_coords;
in vec3 normal;
in flat uint face_index;
out vec4 color;

#include "h/defaults.frag.h"

vec4 base_texture = texture(textures[face_index], tex_coords);
vec3 base_color = base_texture.rgb;
float distance = length(position - player_position);
float flashlight = (FLASHLIGHT_INTENSITY / (distance * FALLOFF)) * toggle_flashlight;

void main()
{
    base_color += sun_direction * SUN_INFLUENCE;
    base_color *= sky_brightness + flashlight;
    base_color = mix(base_color.rgb, sky_color.rgb, sky_color_influence);

    /* reinhard tone mapping */
    float W = 4.0;
    base_color = (base_color * (1.0 + base_color / (W * W))) / (1.0 + base_color);

    /* brightness & contrast */
    base_color *= BRIGHTNESS;
    base_color = (base_color - CONTRAST) * (CONTRAST + 1.0) + CONTRAST;
    color = vec4(base_color * base_texture.rgb, 1.0) * base_texture.a * opacity;
}
