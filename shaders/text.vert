#version 330 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform int row;
uniform int col;
uniform float char_size;
uniform float advance;
uniform float bearing;
uniform mat4 projection;
out vec2 tex_coords;

void main()
{

    gl_Position = projection * vec4(a_pos.x + (advance * char_size), a_pos.y - bearing, 0.0, 1.0);
    tex_coords = vec2(
            (a_tex_coords.s + col) * char_size,
            (a_tex_coords.t + row) * char_size);
}

