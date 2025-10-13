#version 430 core

#define SCALE 0.02

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;
out vec4 vertex_color;

void main()
{
    mat4 mat_offset = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            -0.0025, -0.0025, -0.0025, 1.0);

    gl_Position =
        mat_projection *
        mat_orientation *
        mat_rotation *
        mat_translation *
        mat_offset *
        vec4(a_pos * SCALE, 1.0);

    vertex_color = vec4(floor(a_pos), 1.0) * 20.0;
}
