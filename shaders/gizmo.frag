#version 430 core

in vec4 vertex_color;
out vec4 color;

void main()
{
    color = floor(vertex_color);
}
