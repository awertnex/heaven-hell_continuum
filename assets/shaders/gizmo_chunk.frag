#version 430 core

#define FALLOFF 0.1
#define WHITE_POINT 6.0

uniform vec3 camera_position;
in vec3 vertex_position;
in vec4 chunk_color;
in float camera_distance;
out vec4 color;

void main()
{
    if (chunk_color.a == 0.0) discard;

    vec3 light_position = camera_position.xyz * camera_distance;

    float distance = sqrt(length(vertex_position - light_position));
    float light_color = (chunk_color.r + chunk_color.g + chunk_color.b) / 3.0;
    light_color /= distance * (1.0 - normalize(vec3(1.0, camera_distance, 0.0)).x) * FALLOFF;

    color = vec4(vec3((chunk_color.rgb + light_color) / (distance * 2.0)), 1.0);
    color *= chunk_color.a;

    /* reinhard tone mapping */
    color.rgb = (color.rgb * (1.0 + color.rgb / (WHITE_POINT * WHITE_POINT))) /
        (1.0 + color.rgb);
}
