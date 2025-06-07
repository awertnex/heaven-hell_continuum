#version 330 core

layout (location = 0) in vec3 a_pos;

uniform float ratio;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
mat4 mat_transposition;
float corner_offset = 0.2;
float scale = 0.07;
out vec4 vertex_color;

void main()
{
    mat_transposition = mat4(
            scale / ratio,                  0.0,                    0.0,    0.0,
            0.0,                            scale,                  0.0,    0.0,
            0.0,                            0.0,                    scale,  0.0,
            1.0 - (corner_offset / ratio),  1.0 - corner_offset,    0.0,    1.0);

    vertex_color = vec4(floor(a_pos), 1.0) * 20.0;
    gl_Position = mat_transposition * mat_orientation * mat_rotation * vec4(a_pos, 1.0);
}
