#version 430 core

in vec4 position;
in vec3 normal;
out vec4 color;

#include "h/defaults.frag.h"

vec3 base_color = vec3(0.35, 0.58, 1.41);

void main()
{
    base_color += (sun_direction * SUN_INFLUENCE);
    base_color *= sky_brightness;
    base_color = mix(base_color.rgb, sky_color.rgb, sky_color_influence);

    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    /* brightness & contrast */
    base_color *= BRIGHTNESS;
    base_color = ((base_color - CONTRAST) * (CONTRAST + 1.0)) + CONTRAST;
    color = vec4(base_color, 1.0);
}
