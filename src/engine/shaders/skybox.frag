#version 330 core

precision mediump float;

uniform float time;
uniform vec3 sun_rotation;
const float PI = 3.14159265358979323846;
float intensity = 0.0039;
float mid_day;
float pre_burn;
float burn;
float burn_boost;
float mid_night;
vec4 color;
out vec4 FragColor;

void main()
{
    mid_day =    abs(sin(1.5 * pow(sin(time * PI), 1.0)));
    pre_burn =   abs(sin(pow(sin((time + 0.33) * PI * 1.2), 16.0)));
    burn =       abs(sin(1.5 * pow(sin((time + 0.124) * PI * 1.6), 32.0)));
    burn_boost = abs(pow(sin((time + 0.212) * PI * 1.4), 64.0));
    mid_night =  abs(sin(pow(2.0 * cos(time * PI), 3.0)));

    color = vec4(
            ((mid_day * 171.0) + (burn * 85.0) + (mid_night * 1.0) + (pre_burn * 13.0) + (burn_boost * 76.0)),
            ((mid_day * 229.0) + (burn * 42.0) + (mid_night * 4.0) + (pre_burn * 7.0) + (burn_boost * 34.0)),
            ((mid_day * 255.0) + (burn * 19.0) + (mid_night * 14.0) + (pre_burn * 20.0)), 1.0);

    FragColor = color * intensity;
}

