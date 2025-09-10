#version 330 core

uniform sampler2D texture_font_atlas;
uniform vec3 text_color;
in vec2 tex_coords;
out vec4 color;

void main()
{
    color = vec4(1.0, 1.0, 1.0, texture(texture_font_atlas, tex_coords).r);
}

