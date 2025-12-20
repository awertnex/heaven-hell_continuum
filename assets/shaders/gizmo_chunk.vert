#version 430 core

layout (location = 0) in uint a_pos;
layout (location = 1) in uint a_color;

uniform float gizmo_offset;
out vec3 vs_position;
out vec4 vs_color;

void main()
{
    vs_position = vec3(
            (a_pos >> 0x18) & 0xff,
            (a_pos >> 0x10) & 0xff,
            (a_pos >> 0x08) & 0xff) - gizmo_offset;
    vs_color = vec4(
            (a_color >> 0x18) & 0xff,
            (a_color >> 0x10) & 0xff,
            (a_color >> 0x08) & 0xff,
            (a_color >> 0x00) & 0xff) / 0xff;
}
