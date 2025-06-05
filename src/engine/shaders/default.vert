#version 330 core

layout (location = 0) in vec3 vertex_pos;

/* ---- declarations -------------------------------------------------------- */
uniform mat4 mat_projection;
out vec4 vertex_color;

void main()
{
    vertex_color = vec4(vertex_pos, 1.0) * 20.0;
    gl_Position = mat_projection * vec4(vertex_pos, 1.0);
}
