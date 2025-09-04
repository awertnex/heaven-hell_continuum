#version 330 core

uniform sampler2D texture_text;
uniform vec3 text_color;
in vec2 tex_coords;
out vec4 color;

void main()
{
    vec4 alpha = vec4(1.0, 1.0, 1.0, texture(texture_text, tex_coords).r);
    color = vec4(text_color, 1.0) * alpha;
}

