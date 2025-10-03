#version 430 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_perspective;
uniform vec3 open_cursor;
uniform vec3 offset_cursor;
uniform float size;
out vec3 vertex_position;
//out int vs_data;

void main()
{
    //vs_data = a_data;
    vertex_position = a_pos + open_cursor + offset_cursor;
    gl_Position = mat_perspective * vec4(vertex_position, 1.0);
}
