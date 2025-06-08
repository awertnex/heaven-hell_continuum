#version 330 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_perspective;
out vec4 vertex_position;

void main()
{
    vertex_position = vec4(a_pos, 1.0);
    gl_Position = mat_perspective * vertex_position;
}
