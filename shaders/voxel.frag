#version 430 core

#define FLASHLIGHT_INTENSITY 0.1
#define FALLOFF 0.01
#define SKY_BRIGHTNESS 2.0
#define SKY_INFLUENCE 0.2
uniform vec3 camera_position;
uniform vec3 sun_rotation; /* TODO: use sun_rotation */
uniform vec3 sky_color;
uniform float opacity;
in vec4 gs_diffuse;
in vec3 gs_position;
out vec4 color;

vec4 base_color = vec4(1.0, 0.84, 0.50, 1.0);

void main()
{
    float distance = length(gs_position - camera_position);
    float flashlight = FLASHLIGHT_INTENSITY / (distance * FALLOFF);
    float sky_brightness = SKY_BRIGHTNESS *
        dot(sky_color.rgb, vec3(0.2126, 0.0722, 0.6152));
    float sky_brightness_range = max(sky_brightness + 1.0, 0.001);
    float sky_influence = clamp(sky_brightness * SKY_INFLUENCE, 0.0, 1.0);

    color = base_color * gs_diffuse * opacity;
    color.rgb *= sky_brightness + (flashlight / sky_brightness_range);
    color.rgb = mix(color.rgb, sky_color.rgb, sky_influence);

    /* reinhard tone mapping */
    color.rgb /= color.rgb + vec3(1.0);
}
