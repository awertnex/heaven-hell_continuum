#version 330 core

precision mediump float;

in vec4 vertex_color;
out vec4 FragColor;

void main()
{
    FragColor = vertex_color;
}

