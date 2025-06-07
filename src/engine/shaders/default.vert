#version 330 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_perspective;
out vec4 vertex_pos;
out vec4 vertex_color;

void main()
{
    vertex_pos = vec4(a_pos, 1.0);
    vertex_color = vertex_pos * 20.0;
    gl_Position = mat_perspective * vertex_pos;
}
