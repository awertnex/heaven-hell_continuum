#version 330 core

#ifdef GL_ES
precision lowp float;
#endif // GL_ES

// ---- declarations -----------------------------------------------------------
in vec4 vertex_color;
out vec4 FragColor;

void main()
{
    FragColor = vec4(
            vertex_color.x,
            vertex_color.y - 0.4,
            vertex_color.z - 0.6,
            1.0);
}

