#version 430 core

#define SCALE 0.03

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;

void main()
{
    gl_Position =
        mat_projection *
        mat_orientation *
        mat_rotation *
        mat_translation *
        vec4((a_pos - 0.0025) * SCALE, 1.0);
}
