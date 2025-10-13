#version 430 core

#define SKY_INFLUENCE 0.13

uniform vec3 camera_position;
uniform vec3 sun_rotation; // TODO: use sun_rotation
uniform vec3 sky_color;
uniform float opacity;
in vec4 gs_diffuse;
in vec3 gs_position;
out vec4 color;

vec4 base_color = vec4(0.90, 0.67, 0.40, 1.0) * gs_diffuse;

void main()
{
    float distance = sqrt(length(gs_position - camera_position));
    float falloff = 0.4 / (distance * distance);

    float sky_brightness = sky_color.r + sky_color.g + sky_color.b;
    float flashlight =
        ((base_color.r + sky_color.r) *
        (base_color.g + sky_color.g) *
        (base_color.b + sky_color.b)) / 3.0;
    flashlight /= (distance * falloff);

    vec3 solid_color = (base_color.rgb * sky_brightness) + flashlight;
    color = vec4(((solid_color / (distance * 2.3)) +
        (sky_color.rgb * SKY_INFLUENCE)) * opacity, opacity);
    color.rgb *= distance * falloff;
}
