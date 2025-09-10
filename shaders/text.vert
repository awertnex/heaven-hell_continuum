#version 330 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform float font_size;
//uniform int codepoint;
uniform float ascent;
uniform float descent;
uniform float advance;
uniform int string_index;
out vec2 tex_coords;

void main()
{
    int codepoint = 66;
    int col = (codepoint % 16);
    int row = (codepoint / 16);
    float char_size = 1.0 / 16.0;

    gl_Position = vec4(a_pos * char_size, 0.0, 1.0);
    tex_coords = vec2(
            (a_tex_coords.s * char_size) + (char_size * col),
            ((1.0 - a_tex_coords.t) * char_size) + (char_size * row));
}

