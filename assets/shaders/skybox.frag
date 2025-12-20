#version 430 core

uniform sampler2D texture_sky;
uniform sampler2D texture_horizon;
uniform sampler2D texture_stars;
uniform vec3 sky_color;
in vec3 vs_sun_rotation;
in vec2 tex_coords;
out vec4 color;

void main()
{
    vec4 sky = texture(texture_sky, tex_coords);
    vec4 horizon = texture(texture_horizon, tex_coords);
    horizon.g *= 0.5;
    horizon.b *= 0.3;
    float alpha = (horizon.r + horizon.g + horizon.b) / 3.0;
    vec4 stars = texture(texture_stars, tex_coords);

    color = vec4(sky_color, 1.0) * mix(sky, horizon, alpha) + stars;
    color.rgb *= sky.a;
    color.a = sky.a;
}
