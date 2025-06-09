#version 330 core

layout (location = 0) in vec3 a_pos;

uniform vec3 camera_position;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;

void main()
{
    gl_Position =
        mat_projection *
        mat_orientation *
        mat_rotation *
        vec4(a_pos, 1.0);
}

