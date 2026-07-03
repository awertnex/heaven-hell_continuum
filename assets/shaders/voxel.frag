#version 430 core

#extension GL_ARB_bindless_texture: require

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
uniform int render_distance;
uniform hhc_spotlight flashlight;
in vec3 pos;
in vec2 uv;
in vec3 normal;
in flat uint face_index;
in float block_light;
out vec4 color;

#define USE_SUN_DIRECTION
#define USE_MATH
#define USE_FOG
#define USE_TONE_MAPPING
#include "h/defaults.glsl"

float spotlight_get(hhc_spotlight spotlight, vec3 normal)
{
    vec3 frag_pos = normalize(pos - spotlight.pos);
    float spot_factor = dot(frag_pos, flashlight.direction);
    float epsilon = spotlight.cutoff - (1.0 - spotlight.feather_factor) * spotlight.cutoff;
    float value = clamp((spot_factor - spotlight.cutoff) / epsilon, 0.0, 1.0);

    return value * value * value * (value * (value * 6.0 - 15.0) + 10.0);
}

void main()
{
    float distance = square_length(pos - flashlight.pos);
    float flashlight_intensity = flashlight.intensity / pow(sqrt(distance), 1.0 / FLASHLIGHT_DISTANCE);
    float sky_brightness = (sky_light.r + sky_light.g + sky_light.b) / 3.0;
    float moon_brightness = (moon_light.r + moon_light.g + moon_light.b) / 3.0;

    vec4 texture_base = texture(textures[face_index], uv);
    vec3 color_sky_influence = texture_base.rgb * sky_light *
        SKY_INFLUENCE * block_light * (sky_brightness + moon_brightness);
    vec3 color_sun_influence = texture_base.rgb * sun_direction *
        SUN_INFLUENCE * sky_brightness;
    vec3 color_moon_influence = texture_base.rgb * moon_direction *
        MOON_INFLUENCE * moon_brightness;
    vec3 color_block_light = block_light * (color_sun_influence + color_moon_influence);
    vec3 color_ambient_light = texture_base.rgb * GLOBAL_ILLUMINATION;
    vec3 color_flashlight = FLASHLIGHT_COLOR * texture_base.rgb *
        flashlight_intensity * spotlight_get(flashlight, normal);

    vec3 color_composite =
        color_sky_influence +
        color_block_light +
        color_ambient_light +
        color_flashlight;

    vec3 color_final = mix(color_composite, sky_light,
            fog_linear(distance / float(render_distance * (1.0 - FOG_SOFTNESS / 1.5)),
                render_distance - render_distance * FOG_SOFTNESS,
                render_distance + render_distance * FOG_SOFTNESS));

    color_final = reinhard_tone_mapping(color_final, 30.0);

    color = vec4(color_final, 1.0) * texture_base.a * opacity;
}
