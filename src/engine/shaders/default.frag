#version 330 core

precision mediump float;

uniform vec3 camera_position;
uniform vec3 sun_rotation;
uniform vec3 sky_color;
in vec4 vertex_position;
float distance;
out vec4 FragColor;

float get_distance(vec3 a, vec3 b);

void main()
{
    distance = get_distance(vertex_position.xyz, camera_position);
    FragColor = vec4(sky_color * 0.7, 1.0) + vec4(vertex_position.xyz / (distance * 5.0), 1.0);
}

float get_distance(vec3 a, vec3 b)
{
    return (((a.x - b.x) * (a.x - b.x))
            + ((a.y - b.y) * (a.y - b.y))
            + ((a.z - b.z) * (a.z - b.z)));
}

