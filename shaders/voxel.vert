#version 430 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_perspective;
uniform vec3 open_cursor;
//out int voxel_data;

void main()
{
    gl_Position = mat_perspective * vec4(a_pos + open_cursor, 1.0);
    //voxel_data = a_voxel;
}

