#version 430 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_perspective;
out vec3 vertex_position;

void main()
{
    vertex_position = a_pos;
    gl_Position = mat_perspective * vec4(vertex_position, 1.0);
}
