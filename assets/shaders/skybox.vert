#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;

out vec2 tex_coords;

void main()
{
    tex_coords = a_tex_coords * 0.25f;

    gl_Position =
        mat_projection *
        mat_orientation *
        mat_rotation *
        vec4(a_pos, 1.0);
}
