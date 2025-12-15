#version 430 core

#define FALLOFF 0.1
#define SKY_INFLUENCE 0.13
#define CAMERA_DISTANCE 24.0

uniform vec4 chunk_color;
uniform vec3 camera_position;
uniform vec3 sky_color;
in vec3 vertex_position;
out vec4 color;

void main()
{
    vec3 light_position = camera_position.xyz * CAMERA_DISTANCE;

    float distance = sqrt(length(vertex_position - light_position));

    float sky_brightness = sky_color.r + sky_color.g + sky_color.b;
    float flashlight =
        (chunk_color.r * sky_color.r) +
        (chunk_color.g * sky_color.g) +
        (chunk_color.b * sky_color.b);
    flashlight /= (distance * FALLOFF);

    vec3 solid_color = (chunk_color.rgb * sky_brightness) + flashlight;
    color = vec4(((solid_color / (distance * 2.0)) +
        (sky_color.rgb * SKY_INFLUENCE)), 1.0);
    color *= chunk_color.a;
}
