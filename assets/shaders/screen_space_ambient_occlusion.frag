#version 430 core

#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923

uniform sampler2D texture_world_pos;
uniform sampler2D texture_world_normal;
uniform sampler2D texture_world_albedo_specular;
uniform mat4 mat_projection;
uniform float camera_far;
uniform float camera_near;
uniform bool ssao_enable;
uniform uint time;
in vec2 pos;
in vec2 uv;
out vec4 color;

const float kernel_scale = 1.0 / 64.0;
float ambient_occlusion_intensity = 0.3;

const vec3 sample_kernel[64] = vec3[64](
    vec3(0.010, 0.023, 0.002),
    vec3(0.004, 0.033, 0.010),
    vec3(-0.041, -0.028, 0.048),
    vec3(0.020, -0.029, 0.007),
    vec3(0.027, 0.005, 0.019),
    vec3(-0.035, -0.010, 0.085),
    vec3(-0.034, -0.032, 0.017),
    vec3(0.005, 0.014, 0.028),
    vec3(-0.010, 0.023, 0.005),
    vec3(-0.046, -0.036, 0.040),
    vec3(0.017, 0.033, 0.005),
    vec3(0.020, 0.003, 0.019),
    vec3(-0.051, -0.028, 0.085),
    vec3(0.036, -0.039, 0.014),
    vec3(0.002, 0.013, 0.032),
    vec3(-0.033, 0.008, 0.141),
    vec3(-0.056, -0.049, 0.037),
    vec3(0.016, 0.038, 0.003),
    vec3(0.008, 0.058, 0.018),
    vec3(-0.073, -0.050, 0.087),
    vec3(0.037, -0.054, 0.013),
    vec3(0.052, 0.011, 0.037),
    vec3(-0.067, -0.019, 0.169),
    vec3(-0.068, -0.065, 0.035),
    vec3(0.010, 0.029, 0.058),
    vec3(-0.019, 0.050, 0.012),
    vec3(-0.097, -0.077, 0.086),
    vec3(0.036, 0.071, 0.011),
    vec3(0.045, 0.006, 0.041),
    vec3(-0.111, -0.058, 0.187),
    vec3(0.080, -0.084, 0.031),
    vec3(0.006, 0.028, 0.070),
    vec3(-0.069, 0.021, 0.310),
    vec3(-0.122, -0.106, 0.082),
    vec3(0.036, 0.083, 0.007),
    vec3(0.020, 0.129, 0.039),
    vec3(-0.156, -0.106, 0.190),
    vec3(0.081, -0.116, 0.029),
    vec3(0.112, 0.023, 0.080),
    vec3(-0.139, -0.036, 0.360),
    vec3(-0.144, -0.136, 0.074),
    vec3(0.021, 0.061, 0.121),
    vec3(-0.036, 0.107, 0.026),
    vec3(-0.199, -0.155, 0.180),
    vec3(0.074, 0.146, 0.022),
    vec3(0.094, 0.013, 0.084),
    vec3(-0.218, -0.112, 0.377),
    vec3(0.160, -0.165, 0.063),
    vec3(0.012, 0.056, 0.138),
    vec3(-0.128, 0.046, 0.601),
    vec3(-0.234, -0.203, 0.161),
    vec3(0.069, 0.158, 0.015),
    vec3(0.042, 0.247, 0.075),
    vec3(-0.291, -0.195, 0.361),
    vec3(0.152, -0.214, 0.055),
    vec3(0.210, 0.044, 0.148),
    vec3(-0.248, -0.058, 0.661),
    vec3(-0.261, -0.246, 0.137),
    vec3(0.056, 0.163, 0.000),
    vec3(-0.057, 0.198, 0.050),
    vec3(-0.351, -0.273, 0.323),
    vec3(0.131, -0.256, 0.041),
    vec3(0.169, 0.025, 0.147),
    vec3(-0.373, -0.187, 0.658)
);

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
        kernel_sample = pos_vec.xyz + tbn * sample_kernel[i] * radius;
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
    ambient_occlusion *= color_albedo.a;

    color = color_albedo;
    color.rgb *= ambient_occlusion;
}
