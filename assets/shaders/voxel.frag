#version 430 core

#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 g_color;
layout(location = 1) out vec4 g_pos;
layout(location = 2) out vec4 g_normal;
layout(location = 3) out vec4 g_albedo_specular;

layout(std430, binding = 2) readonly buffer ssbo_textures
{
    sampler2D textures[];
};

struct hhc_spotlight
{
    vec3 pos;
    vec3 direction;
    float cutoff;
    float feather_factor;
    float intensity;
};

uniform sampler2D texture_block;
uniform float opacity;
uniform vec3 camera_position;
uniform int render_distance;
uniform hhc_spotlight flashlight;
in vec4 pos;
in vec4 pos_view;
in vec2 uv;
in vec3 normal;
in vec4 normal_view;
in flat uint face_index;
in float block_light;

#define USE_SUN_DIRECTION
#define USE_MATH
#define USE_FOG
#define USE_TONE_MAPPING
#include "h/defaults.glsl"

float spotlight_get(hhc_spotlight spotlight, vec3 normal)
{
    vec3 frag_pos = normalize(pos.xyz - spotlight.pos);
    float spot_factor = dot(frag_pos, flashlight.direction);
    float epsilon = spotlight.cutoff - (1.0 - spotlight.feather_factor) * spotlight.cutoff;
    float value = clamp((spot_factor - spotlight.cutoff) / epsilon, 0.0, 1.0);

    return value * value * value * (value * (value * 6.0 - 15.0) + 10.0) * spotlight.intensity;
}

void main()
{
    float distance = square_length(pos.xyz - flashlight.pos);
    float flashlight_attenuation = 1.0 / pow(sqrt(distance), 1.0 / FLASHLIGHT_DISTANCE);
    float sky_brightness = (sky_light.r + sky_light.g + sky_light.b) / 3.0;
    float moon_brightness = (moon_light.r + moon_light.g + moon_light.b) / 3.0;

    vec4 albedo = texture(textures[face_index], uv);
    vec3 color_sky_influence = albedo.rgb * sky_light *
        SKY_INFLUENCE * block_light * (sky_brightness + moon_brightness);
    vec3 color_sun_influence = albedo.rgb * sun_direction *
        SUN_INFLUENCE * sky_brightness;
    vec3 color_moon_influence = albedo.rgb * moon_direction *
        MOON_INFLUENCE * moon_brightness;
    vec3 color_block_light = block_light * (color_sun_influence + color_moon_influence);
    vec3 color_ambient_light = albedo.rgb * GLOBAL_ILLUMINATION;
    vec3 color_flashlight = FLASHLIGHT_COLOR * albedo.rgb *
        spotlight_get(flashlight, normal) * flashlight_attenuation;

    vec3 color_composite =
        color_sky_influence +
        color_block_light +
        color_ambient_light +
        color_flashlight;

    vec3 color_final = mix(color_composite, sky_light,
            fog_linear(distance / float(render_distance * (1.0 - FOG_SOFTNESS / 1.5)),
                render_distance - render_distance * FOG_SOFTNESS,
                render_distance + render_distance * FOG_SOFTNESS));

    color_final = reinhard_tone_mapping(color_final, WHITE_POINT);

    g_color = vec4(color_final, opacity) * albedo.a * opacity;
    g_pos = pos_view;
    g_normal = vec4(normalize(normal_view.xyz), 1.0);
    g_albedo_specular = g_color;
    g_albedo_specular = albedo;
}
