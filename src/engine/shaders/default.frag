#version 330 core

precision mediump float;

// ---- declarations -----------------------------------------------------------
in vec4 vertex_color;
out vec4 FragColor;

void main()
{
    FragColor = vec4(
            vertex_color.x - 1,
            vertex_color.y - 1,
            vertex_color.z - 1,
            1.0);
}

