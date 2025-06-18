#version 330 core

uniform vec3 camera_position;
uniform vec3 sun_rotation; // TODO: use sun_rotation
uniform vec3 sky_color;
in vec3 vertex_position;
out vec4 FragColor;

float get_distance(vec3 a, vec3 b)
{
    return sqrt(pow(a.x - b.x, 2.0) + pow(a.y - b.y, 2.0) + pow(a.z - b.z, 2.0));
}

vec4 base_color = vec4(0.3, 0.15, 0.03, 1.0);
float distance;
float sky_influence = 0.13;
float sky_brightness;
float flashlight;
void main()
{
    distance = get_distance(vertex_position, camera_position);
    distance /= sqrt(distance);

    sky_brightness = sky_color.r + sky_color.g + sky_color.b;
    flashlight = (
            (base_color.r * sky_color.r) +
            (base_color.g * sky_color.g) +
            (base_color.b * sky_color.b)) / sky_brightness;
    flashlight /= distance;

    FragColor = (((base_color * (1.0 + sky_brightness)) + flashlight) / distance) +
        (vec4(sky_color, 1.0) * sky_influence);
}

