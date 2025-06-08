#version 330 core

precision mediump float;

uniform vec3 camera_position;
in vec4 vertex_position;
float distance;
out vec4 FragColor;

float get_distance(vec3 a, vec3 b);

void main()
{
    distance = get_distance(vertex_position.xyz, camera_position);
    FragColor = vec4(vertex_position.xyz / (distance * 2.0), 1.0);
}

float get_distance(vec3 a, vec3 b)
{
    return (((a.x - b.x) * (a.x - b.x))
            + ((a.y - b.y) * (a.y - b.y))
            + ((a.z - b.z) * (a.z - b.z)));
}

