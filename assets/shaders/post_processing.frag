#version 430 core

#define ABERRATION_NARROWNESS 0.003
#define ABERRATION_INTENSITY 0.2
#define VIGNETTE_NARROWNESS 0.6
#define VIGNETTE_INTENSITY 0.1
#define GRAIN_SAMPLE_SIZE 0.1
#define GRAIN_INTENSITY 0.1
#define GRAIN_SEED_R 300
#define GRAIN_SEED_G 600
#define GRAIN_SEED_B 900

uniform sampler2D texture_screen;
uniform uint time;
in vec2 vs_pos;
in vec2 vs_tex_coords;
out vec4 color;

void main()
{ 
    /* ---- aberration ------------------------------------------------------ */
    vec2 aberration = (vs_pos * ABERRATION_NARROWNESS) *
        pow(length(vs_pos), 3.0);
    vec3 aberration_color = vec3(
            texture(texture_screen, vs_tex_coords +
                (aberration * ABERRATION_INTENSITY)).r,
            texture(texture_screen, vs_tex_coords).g,
            texture(texture_screen, vs_tex_coords -
                (aberration * ABERRATION_INTENSITY)).b);

    /* ---- vignette -------------------------------------------------------- */
    float vignette = pow(length(vs_pos) * VIGNETTE_NARROWNESS, 3.0);
    vignette = clamp(vignette, 0.0, 1.0) * VIGNETTE_INTENSITY;

    /* ---- film grain ------------------------------------------------------ */
    vec2 grain_pos = (vs_pos * 0.5) + 0.5;
    vec3 grain = vec3(
            dot(grain_pos * (time + GRAIN_SEED_R) * GRAIN_SAMPLE_SIZE,
                vec2(-grain_pos.y, 78.233)),
            dot(grain_pos * (time + GRAIN_SEED_G) * GRAIN_SAMPLE_SIZE,
                vec2(-grain_pos.y, 78.233)),
            dot(grain_pos * (time + GRAIN_SEED_B) * GRAIN_SAMPLE_SIZE,
                vec2(-grain_pos.y, 78.233)));
    grain = vec3(
            sin(grain.r) * 43758.5453,
            cos(grain.g) * 43758.5453,
            -sin(grain.b) * 43758.5453);
    grain = ((fract(grain) * 2.0) - 1.0) * GRAIN_INTENSITY;

    /* ---- final ----------------------------------------------------------- */
    color.rgb = aberration_color;
    color.rgb *= (1.0 - grain);
    color.rgb -= vignette;
    color.rgb = mix(color.rgb, color.rgb * 2.0, vignette);
}
