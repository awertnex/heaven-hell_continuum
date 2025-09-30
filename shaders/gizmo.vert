#version 430 core

layout (location = 0) in vec3 a_pos;

uniform float ratio;
uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;
out vec4 vertex_color;

void main()
{
    float scale = 0.04;

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
        vec4(a_pos * scale, 1.0);

    vertex_color = vec4(floor(a_pos), 1.0) * 20.0;
}

