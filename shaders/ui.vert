#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform vec2 position;
uniform ivec2 size;
uniform vec2 ndc_scale;
uniform ivec2 alignment;
out vec2 tex_coords;

void main()
{
    vec2 vs_alignment = vec2(alignment.x, -alignment.y);
    vec2 vs_position = vec2(position.x, -position.y) * ndc_scale;
    vec2 scale = vec2(size / 2.0) * ndc_scale;

    gl_Position =
        vec4(-1.0, 1.0, 0.0, 0.0) +
        (vec4(a_pos + vs_alignment, 0.0, 1.0) * vec4(scale, 0.0, 1.0)) +
        vec4(vs_position, 0.0, 0.0);
    tex_coords = a_tex_coords;
}
