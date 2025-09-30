#version 430 core

//uniform vec4 voxel_color;
out vec4 color;

vec4 base_color = vec4(0.3, 0.15, 0.03, 1.0);

void main()
{
//    color = voxel_color * distance;
    color = base_color;
}

