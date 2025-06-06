#version 330 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_projection;
out vec4 vertex_color;

void main()
{
    vertex_color = vec4(a_pos, 1.0) * 20.0;
    gl_Position = mat_projection * vec4(a_pos, 1.0);
}
