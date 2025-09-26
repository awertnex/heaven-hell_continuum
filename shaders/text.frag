#version 330 core

in vec2 tex_coords;
out vec4 color;
uniform sampler2D texture_font_atlas;
uniform vec4 text_color;

void main()
{
    color = text_color * texture(texture_font_atlas, tex_coords).r;
    color = text_color;
}

