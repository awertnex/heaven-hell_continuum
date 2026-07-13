#version 430 core

#define CUBE_SCALE 0.9
#define GIZMO_SCALE 150.0
#define RENDER_DISTANCE_MAX 32.0
#define RENDER_DISTANCE_MIN 2.0
#define CAMERA_DISTANCE_PADDING 3.0

layout (location = 0) in vec3 a_pos;
layout (location = 1) in uvec2 a_data;
layout (location = 2) in float a_offset;

uniform float gizmo_offset;
uniform ivec2 render_size;
uniform int chunk_buf_diameter;
uniform mat4 mat_projection;
out vec3 vertex_position;
out vec4 chunk_color;
out float camera_distance;
vec2 ndc_scale = 2.0 / vec2(render_size);
float inv_255 = 1.0 / 255.0;
float gizmo_scale = GIZMO_SCALE * (1.0 / chunk_buf_diameter) * ndc_scale.y;

void main()
{
    uint alpha = a_data.y & 0xff;

    vertex_position = a_pos * CUBE_SCALE + vec3(
            (a_data.x >> 0x18) & 0xff,
            (a_data.x >> 0x10) & 0xff,
            ((a_data.x >> 0x08) & 0xff)) - gizmo_offset;

    chunk_color = vec4(
            (a_data.y >> 0x18) & 0xff,
            (a_data.y >> 0x10) & 0xff,
            (a_data.y >> 0x08) & 0xff,
            alpha) * (alpha * inv_255) * inv_255;

    camera_distance = float(chunk_buf_diameter) / 2.0 + CAMERA_DISTANCE_PADDING;

    float offset = 1.0;
    if (bool(a_offset != 0.0))
        offset = 0.0;

    mat4 mat = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, a_offset, 1.0);

    gl_Position =
        mat *
        mat_projection *
        vec4(vertex_position * gizmo_scale, 1.0);
}
