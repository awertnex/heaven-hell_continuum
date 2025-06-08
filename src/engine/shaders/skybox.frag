#version 330 core

precision mediump float;

uniform vec3 sun_rotation;
uniform vec3 sky_color;
out vec4 FragColor;

void main()
{
    FragColor = vec4(sky_color, 1.0);
}

