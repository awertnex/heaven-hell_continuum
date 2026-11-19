#version 430 core

#define ABERRATION_NARROWNESS 0.003
#define ABERRATION_INTENSITY 0.2

#define VIGNETTE_NARROWNESS 0.6
#define VIGNETTE_INTENSITY 0.2
#define VIGNETTE_RICHNESS 2.0

#define GRAIN_INTENSITY 0.1
#define GRAIN_SATURATION 0.5

#define COLOR_RICHNESS 0.6
#define SATURATION 1.0

#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923

uniform sampler2D texture_world;
uniform sampler2D texture_hud;
uniform uint time;
in vec2 pos;
in vec2 uv;
out vec4 color;

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

vec3 vignette_get(vec2 frag_pos, vec3 color_src, float narrowness, float intensity, float richness)
{
    float vignette = length(frag_pos) * narrowness;
    vignette = vignette * vignette * vignette;
    vignette = clamp(vignette, 0.0, 1.0) * intensity;
    vec3 color_rich = pow(color_src, vec3(richness));
    return mix(color_src, color_rich, vignette) - vignette;
}

vec3 color_enrich(vec3 color_src, float richness)
{
    vec3 color_rich = color_src * color_src * color_src * (color_src * (color_src * 6.0 - 15.0) + 10.0);
    return mix(color_src, color_rich, richness);
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

void main()
{
    /* ---- base ------------------------------------------------------------ */

    vec4 color_albedo = aberration_get(texture_world, pos, uv,
            ABERRATION_NARROWNESS, ABERRATION_INTENSITY);
    vec4 color_ui = texture(texture_hud, uv);

    /* ---- effects --------------------------------------------------------- */

    vec3 grain = 1.0 + grain_get(pos, time, GRAIN_INTENSITY, GRAIN_SATURATION);

    /* ---- final ----------------------------------------------------------- */

    color.rgb = color_albedo.rgb * color_albedo.a;

    color.rgb = vignette_get(pos, color.rgb, VIGNETTE_NARROWNESS, VIGNETTE_INTENSITY, VIGNETTE_RICHNESS);
    color.rgb = color_enrich(color.rgb, COLOR_RICHNESS);

    color.rgb = mix(color.rgb, color_ui.rgb, color_ui.a);

    color.rgb *= grain;
    color.rgb = saturation_get(color.rgb, SATURATION);
}
