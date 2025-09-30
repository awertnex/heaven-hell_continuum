#version 430 core

uniform vec3 sun_rotation;
uniform vec3 sky_color;
out vec4 color;

void main()
{
    color = vec4(sky_color, 1.0);
}

