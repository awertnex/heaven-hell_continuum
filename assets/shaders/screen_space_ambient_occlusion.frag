#version 430 core

#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923

uniform sampler2D texture_world_pos;
uniform sampler2D texture_world_normal;
uniform sampler2D texture_world_albedo_specular;
uniform mat4 mat_projection;
uniform float camera_far;
uniform float camera_near;
uniform vec3 ssao_sample[64];
uniform bool ssao_enable;
uniform uint time;
in vec2 pos;
in vec2 uv;
out vec4 color;

const float kernel_scale = 1.0 / 64.0;
float ambient_occlusion_intensity = 0.3;

/* ---- implementation ------------------------------------------------------ */

float depth_linearize(float depth, float far, float near)
{
    depth = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - depth * (far - near));
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
        vec2 frag_pos, vec2 frag_uv, float intensity)
{
    vec4 pos_vec = texture(sampler_pos, frag_uv);
    vec3 normal_vec = texture(sampler_normal, frag_uv).xyz;
    vec3 random_vec = grain_get(frag_pos, time, 1.0, 1.0);
    vec4 depth_vec = vec4(0.0);

    vec3 tangent = normalize(random_vec - normal_vec * dot(random_vec, normal_vec));
    vec3 bitangent = normalize(cross(normal_vec, tangent));
    mat3 tbn = mat3(tangent, bitangent, normal_vec);

    vec3 kernel_sample = vec3(0.0);
    float radius = 2.0;
    float bias = 0.025;
    vec4 offset = vec4(0.0);
    float depth_a = mix(camera_far, pos_vec.z, pos_vec.w);
    float depth_b = 0.0;
    float range_check = 0.0;
    float sum = 0.0;
    int i = 0;

    for(; i < 64; ++i)
    {
        kernel_sample = pos_vec.xyz + tbn * ssao_sample[i] * radius;
        offset = mat_projection * vec4(kernel_sample, 1.0);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        depth_vec = texture(sampler_pos, offset.xy);
        depth_b = mix(camera_far, depth_vec.z, depth_vec.w);

        range_check = smoothstep(0.0, 1.0, radius / length(depth_a - depth_b));
        sum += (depth_b >= kernel_sample.z + bias ? kernel_scale : 0.0) * range_check;
    }

    return 1.0 - clamp(sum * intensity, 0.0, 1.0);
}

void main()
{
    vec4 color_albedo = texture(texture_world_albedo_specular, uv);
    float ambient_occlusion = ssao_enable ? ambient_occlusion_get(texture_world_pos,
            texture_world_normal, pos, uv, ambient_occlusion_intensity) : 1.0;

    color = color_albedo * color_albedo.a;
    color.rgb *= ambient_occlusion;
}
