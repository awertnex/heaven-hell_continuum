#version 430 core

#define FALLOFF 0.3
#define SKY_INFLUENCE 0.13

uniform vec3 camera_position;
uniform vec3 sun_rotation; /* TODO: use sun_rotation */
uniform vec3 sky_color;
in vec3 vertex_position;
out vec4 color;

vec4 base_color = vec4(0.3, 0.15, 0.03, 1.0);

void main()
{
    float distance = length(vertex_position - camera_position);
    distance /= sqrt(distance);

    float sky_brightness = sky_color.r + sky_color.g + sky_color.b;
    float flashlight = (
            (base_color.r * sky_color.r) +
            (base_color.g * sky_color.g) +
            (base_color.b * sky_color.b)) / 3.0;
    flashlight /= (distance * FALLOFF);

    color = (((base_color * (1.0 + sky_brightness)) + flashlight) / (distance * 2.0)) +
        (vec4(sky_color, 1.0) * SKY_INFLUENCE);
}
