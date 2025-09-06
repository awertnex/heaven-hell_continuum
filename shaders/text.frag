#version 330 core

uniform sampler2D texture_text;
uniform vec3 text_color;
in vec2 tex_coords;
out vec4 color;

void main()
{
    color = vec4(texture(texture_text, tex_coords).r);
}

