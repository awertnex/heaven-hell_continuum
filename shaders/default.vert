#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform vec3 offset;
uniform vec3 scale;
uniform mat4 mat_perspective;
out vec3 vs_position;
out vec3 vs_normal;

void main()
{
    vs_position = (a_pos * scale) + offset;
    vs_position.xy -= (scale.xy / 2.0);
    vs_normal = a_normal;
    gl_Position = mat_perspective * vec4(vs_position, 1.0);
}
