#version 330 core

layout (location = 0) in int a_voxel;

out int voxel_data;

void main()
{
    voxel_data = a_voxel;
}

