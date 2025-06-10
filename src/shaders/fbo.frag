#version 330 core

uniform sampler2D screen_texture;
in vec2 tex_coords;
out vec4 FragColor;

void main()
{ 
    FragColor = texture(screen_texture, tex_coords);
}

