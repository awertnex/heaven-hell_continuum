#version 430 core

uniform sampler2D texture_screen;
uniform vec4 tint;
in vec2 tex_coords;
out vec4 color;

void main()
{ 
    color = texture(texture_screen, tex_coords) * tint;
}
