#version 430 core

uniform vec3 camera_position;
uniform vec3 sun_rotation; // TODO: use sun_rotation
uniform vec3 sky_color;
uniform float opacity;
in vec3 vertex_position;
out vec4 color;

vec4 base_color = vec4(0.3, 0.15, 0.03, 1.0);
float sky_influence = 0.13;

void main()
{
    float distance = length(vertex_position - camera_position);
    distance /= sqrt(distance);
    distance /= sqrt(distance);

    float falloff = 0.3f;
    float sky_brightness = sky_color.r + sky_color.g + sky_color.b;
    float flashlight =
        (base_color.r * sky_color.r) +
        (base_color.g * sky_color.g) +
        (base_color.b * sky_color.b);
    flashlight /= (distance * falloff);

    vec3 solid_color = (base_color.rgb * sky_brightness) + flashlight;
    color = vec4(((solid_color / (distance * 2.0)) +
        (sky_color.rgb * sky_influence)) * opacity, opacity);
}
