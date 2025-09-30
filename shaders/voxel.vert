#version 330 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_perspective;
uniform vec3 open_cursor;
out vec3 vertex_position;
//out int voxel_data;

void main()
{
    vertex_position = a_pos;
    gl_Position = mat_perspective * vec4(vertex_position + open_cursor, 1.0);
    //voxel_data = a_voxel;
}

