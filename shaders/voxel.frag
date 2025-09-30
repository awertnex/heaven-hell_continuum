#version 330 core

uniform vec3 camera_position;
uniform vec4 voxel_color;
in vec3 vertex_position;
out vec4 color;

float get_distance(vec3 a, vec3 b)
{
    return sqrt(pow(a.x - b.x, 2.0) + pow(a.y - b.y, 2.0) + pow(a.z - b.z, 2.0));
}

void main()
{
    float distance = get_distance(vertex_position, camera_position);
    distance /= sqrt(distance);

    color = voxel_color * distance;
}

