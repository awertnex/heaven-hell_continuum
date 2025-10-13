#version 430 core

#define FLASHLIGHT_INTENSITY 15.0
#define FALLOFF 2.0
#define SKY_BRIGHTNESS 2.0
#define SKY_INFLUENCE 0.2
#define SUN_INFLUENCE 0.5
#define BRIGHTNESS 1.0
#define CONTRAST 0.5

uniform vec3 camera_position;
uniform vec3 sun_rotation;
uniform vec3 sky_color;
in vec4 vs_position;
in vec3 vs_normal;
out vec4 color;

vec3 base_color = vec3(0.35, 0.58, 1.41);

void main()
{
    float distance = length(vs_position.xyz - camera_position);
    float flashlight = FLASHLIGHT_INTENSITY / (distance * FALLOFF);
    float sky_brightness = SKY_BRIGHTNESS *
        dot(sky_color.rgb, vec3(0.2126, 0.0722, 0.6152));
    float sky_color_influence = clamp(sky_brightness * SKY_INFLUENCE, 0.0, 1.0);
    float sun_direction = dot(vs_normal, sun_rotation);
    sun_direction = (sun_direction + 1.0) / 2.0;

    base_color += (sun_direction * SUN_INFLUENCE);
    base_color *= sky_brightness + flashlight;
    base_color = mix(base_color.rgb, sky_color.rgb, sky_color_influence);

    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    /* brightness & contrast */
    base_color *= BRIGHTNESS;
    base_color = ((base_color - CONTRAST) * (CONTRAST + 1.0)) + CONTRAST;
    color = vec4(base_color, 1.0);
}
