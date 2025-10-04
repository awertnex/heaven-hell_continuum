#version 430 core

uniform sampler2D texture_screen;
in vec2 vs_pos;
in vec2 tex_coords;
out vec4 color;

#define VIGNETTE 0.2

void main()
{ 
    float vignette = 1.0 - pow(length(vs_pos) * VIGNETTE, 3.0);
    vignette = clamp(vignette, 0.0, 1.0);
    color = vec4(vec3(texture(texture_screen, tex_coords) * vignette), 1.0);
}
