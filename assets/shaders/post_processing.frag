#version 430 core

#include "h/post_processing.glsl"

#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923

uniform sampler2D texture_skybox;
uniform sampler2D texture_world_pos;
uniform sampler2D texture_world_normal;
uniform sampler2D texture_world_albedo_specular;
uniform sampler2D texture_hud;
uniform uint time;
uniform vec3 ssao_sample[64];
uniform mat4 mat_projection;
in vec2 vs_pos;
in vec2 vs_uv;
out vec4 color;
const float occlusion_scale = 1.0 / 64.0;

/* ---- settings ------------------------------------------------------------ */

float setting_saturation = 1.0;
float setting_vignette_color_richness = 2.0;
float setting_grain_intensity = 0.2;
float setting_color_richness = 0.75;

/* ---- implementation ------------------------------------------------------ */

vec3 saturation_get(vec3 color_src, float saturation)
{
    vec3 color_monochrome = vec3((color_src.r + color_src.g + color_src.b) / 3.0);
    return mix(color_monochrome, color_src, saturation);
}

vec4 aberration_get(sampler2D sampler, vec2 frag_pos, vec2 frag_uv,
        float narrowness, float intensity)
{
    float length = length(frag_pos);
    vec2 aberration = frag_pos * narrowness * length * length * length * intensity;
    vec4 final = texture(sampler, frag_uv);
    final.r = texture(sampler, frag_uv + aberration).r;
    final.b = texture(sampler, frag_uv - aberration).b;
    return final;
}

vec3 grain_get(vec2 frag_pos, uint seed, float intensity, float chroma)
{
    vec2 grain_pos = frag_pos * 0.5 + 0.5;
    float grain_size = 0.1;
    float base = fract(dot(grain_pos * seed * grain_size, vec2(-grain_pos.y, 78.233)));
    float drift = 43758.5453324;
    vec3 grain = vec3(
            sin(base),
            sin(base + HALF_PI),
            sin(base + PI)) * drift;
    grain = fract(grain) * 2.0 - 1.0;
    return mix(vec3(grain.r), grain, chroma) * intensity;
}

float ambient_occlusion_get(sampler2D sampler_pos, sampler2D sampler_normal,
        vec2 frag_pos, vec2 frag_uv, float intensity_factor)
{
    vec3 pos_vec = texture(sampler_pos, frag_uv).rgb;
    vec3 normal_vec = texture(sampler_normal, frag_uv).rgb;
    vec3 random_vec = grain_get(frag_pos, time, 1.0, 1.0);

    vec3 tangent = normalize(random_vec - normal_vec * dot(random_vec, normal_vec));
    vec3 bitangent = cross(normal_vec, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal_vec);
    float occlusion = 0.0;
    float radius = 0.5;
    float bias = 0.025;
    vec3 sample_curr = vec3(0.0);
    vec4 offset = vec4(0.0);
    float sample_depth = 0.0;
    float range_check = 0.0;
    int i = 0;

    for(; i < 64; ++i)
    {
        sample_curr = pos_vec + tbn * ssao_sample[i] * radius;
        offset = vec4(sample_curr, 1.0);
        offset = mat_projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = clamp(offset.xyz * 0.5 + 0.5, 0.0, 1.0);
        sample_depth = texture(sampler_pos, offset.xy).z;

        range_check = smoothstep(0.0, 1.0, radius / abs(pos_vec.z - sample_depth));
        occlusion += (sample_depth >= sample_curr.z + bias ? occlusion_scale : 0.0) * range_check;
    }

    return occlusion * intensity_factor;
}

void main()
{
    /* ---- base ------------------------------------------------------------ */

    vec4 color_skybox = aberration_get(texture_skybox, vs_pos, vs_uv,
            ABERRATION_NARROWNESS, ABERRATION_INTENSITY);
    vec4 color_albedo = aberration_get(texture_world_albedo_specular, vs_pos, vs_uv,
            ABERRATION_NARROWNESS, ABERRATION_INTENSITY);
    vec3 color_albedo_cubed = color_albedo.rgb * color_albedo.rgb * color_albedo.rgb;
    vec3 global_illumination = color_albedo.rgb;
    vec4 color_ui = texture(texture_hud, vs_uv);

    /* ---- effects --------------------------------------------------------- */

    vec3 color_rich = vec3(0.0);
    vec3 grain = 1.0 + grain_get(vs_pos, time, setting_grain_intensity, 1.0);
    float ambient_occlusion = ambient_occlusion_get(texture_world_pos, texture_world_normal, vs_pos, vs_uv, 1.0);
    float vignette = pow(length(vs_pos) * VIGNETTE_NARROWNESS, 3.0);
    vignette = clamp(vignette, 0.0, 1.0) * VIGNETTE_INTENSITY;

    /* ---- layering -------------------------------------------------------- */

    color_albedo.rgb = mix(color_albedo.rgb, color_albedo_cubed, ambient_occlusion);
    color_albedo = mix(color_skybox, vec4(color_albedo.rgb, 1.0), color_albedo.a);
    global_illumination = color_albedo.rgb;

    /* ---- final ----------------------------------------------------------- */

    color.rgb -= vignette;
    color.rgb = mix(color_albedo.rgb, color_albedo.rgb * setting_vignette_color_richness, vignette);

    color_rich = color.rgb * color.rgb * color.rgb * (color.rgb * (color.rgb * 6.0 - 15.0) + 10.0);
    color.rgb = mix(color.rgb, color_rich, setting_color_richness);

    color.rgb = mix(color.rgb, color_ui.rgb, color_ui.a);

    color.rgb *= grain;

    color.rgb = saturation_get(color.rgb, setting_saturation);
    color.rgb = color_albedo.rgb;
}
