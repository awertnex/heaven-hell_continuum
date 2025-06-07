#version 330 core

precision mediump float;

uniform vec3 camera_position;
in vec4 vertex_pos;
in vec4 vertex_color;
out vec4 FragColor;
float distance;

float get_distance(vec3 a, vec3 b);

void main()
{
    distance = get_distance(vertex_pos.xyz, camera_position);
    FragColor = vec4(vertex_color.xyz / (distance * 8.0), 1.0) * 0.1;
}

float get_distance(vec3 a, vec3 b)
{
    return (((a.x - b.x) * (a.x - b.x))
            + ((a.y - b.y) * (a.y - b.y))
            + ((a.z - b.z) * (a.z - b.z)));
}

