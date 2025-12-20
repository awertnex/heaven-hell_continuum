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
uniform int render_distance;
in vec3 vertex_position;
in vec2 tex_coords;
in vec3 normal;
in flat uint face_index;
in float block_light;
out vec4 color;

#include "h/defaults.frag.h"

float square_length(const vec3 v);
float fog_linear(const float distance, float min, float max);

void main()
{
    float distance = square_length(vertex_position - player_position);
    float flashlight = (1.0 / distance) * FLASHLIGHT_INTENSITY * toggle_flashlight;

    vec4 texture_base = texture(textures[face_index], tex_coords);
    vec3 color_sky_influence = texture_base.rgb * sky_color * SKY_INFLUENCE * block_light;
    vec3 color_sun_influence = texture_base.rgb * sun_direction * SUN_INFLUENCE;
    vec3 color_block_light = block_light * color_sun_influence;
    vec3 color_flashlight = texture_base.rgb * flashlight;
    vec3 color_composite = 
        (color_block_light +
         color_sky_influence +
         color_flashlight) / 3.0;
    vec3 color_final = mix(color_composite, sky_color,
            fog_linear(distance / float(render_distance * (1.0 - FOG_SOFTNESS / 1.5)),
                render_distance - render_distance * FOG_SOFTNESS,
                render_distance + render_distance * FOG_SOFTNESS));

    /* reinhard tone mapping */
    float W = 15.0;
    color_final = (color_final * (1.0 + color_final / (W * W))) / (1.0 + color_final);

    color = vec4(color_final, 1.0) * texture_base.a * opacity;
}

float square_length(const vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float fog_linear(const float distance, float min, float max)
{
    return 1.0 - clamp((max - distance) / (max - min), 0.0, 1.0);
}
