#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform vec2 position;
uniform ivec2 size;
uniform ivec2 render_size;
uniform vec2 ndc_scale;
out vec2 tex_coords;

void main()
{
    mat4 mat_offset = mat4(
            ndc_scale.x, 0.0, 0.0, 0.0,
            0.0, ndc_scale.y, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            -1.0, 1.0, 0.0, 1.0);
    vec2 vs_position = vec2(position.x, -position.y);
    vec2 scale = size / 2.0;

    gl_Position = mat_offset *
        vec4((a_pos + (vs_position / scale)) * scale, 0.0, 1.0); 

    tex_coords = a_tex_coords;
}
