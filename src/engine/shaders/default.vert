#version 330 core

layout (location = 0) in vec3 vertex_pos;

// ---- declarations -----------------------------------------------------------
uniform mat4 mat_perspective;
out vec4 vertex_color;
vec4 position;

void main()
{
    vertex_color = vec4(vertex_pos, 1.0) * 10.0;

    position = mat_perspective * vec4(vertex_pos, 1.0);
    gl_Position = vec4(position.xyz / position.w, 1.0);
}
