#version 330 core

layout (location = 0) in vec4 a_data;

out vec2 tex_coords;

void main()
{
    gl_Position = vec4(a_data.xy, 0.0, 1.0);
    tex_coords = a_data.zw;
}

