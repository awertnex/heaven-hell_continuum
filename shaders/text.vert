#version 330 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform int row;
uniform int col;
uniform float char_size;
uniform vec2 glyph_size;
uniform vec2 ndc_size;
uniform vec2 offset;
uniform float advance;
uniform float bearing;
out vec2 tex_coords;

void main()
{
    mat4 projection =
        mat4(
                glyph_size.x, 0.0, 0.0, 0.0,
                0.0, glyph_size.y, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                offset.x + (advance * ndc_size.x), offset.y + (bearing * ndc_size.y), 0.0, 1.0
            );


    gl_Position = projection * vec4(a_pos, 0.0, 1.0);

    tex_coords = vec2(
            (a_tex_coords.s + col) * char_size,
            (a_tex_coords.t + row) * char_size);
}

