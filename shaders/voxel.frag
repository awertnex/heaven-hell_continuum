#version 430 core

#define FLASHLIGHT_INTENSITY 30.0
#define FALLOFF 5.0
#define SKY_BRIGHTNESS 2.0
#define SKY_INFLUENCE 0.25
#define SUN_INFLUENCE 0.5
#define BRIGHTNESS 1.0
#define CONTRAST 0.5

uniform sampler2D texture_block;
uniform vec3 player_position;
uniform vec3 sun_rotation;
uniform vec3 sky_color;
uniform float opacity;
in vec3 gs_position;
in vec2 gs_tex_coords;
in vec3 gs_normal;
out vec4 color;

vec3 base_color = vec3(1.0, 0.74, 0.41);
vec4 base_texture = texture(texture_block, gs_tex_coords);

void main()
{
    float distance = length(gs_position - player_position);
    float flashlight = FLASHLIGHT_INTENSITY / (distance * FALLOFF);
    float sky_brightness = SKY_BRIGHTNESS *
        dot(sky_color.rgb, vec3(0.2126, 0.0722, 0.6152));
    float sky_color_influence = clamp(sky_brightness * SKY_INFLUENCE, 0.0, 1.0);
    float sun_direction = dot(gs_normal, sun_rotation);
    sun_direction = (sun_direction + 1.0) / 2.0;

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
