#version 430 core

#define CUBE_SCALE 0.9
#define GIZMO_SCALE 150.0
#define RENDER_DISTANCE_MAX 32.0
#define RENDER_DISTANCE_MIN 1.0
#define CAMERA_DISTANCE_PADDING 3.0

layout (location = 0) in vec3 a_pos;
layout (location = 1) in ivec4 a_chunk_pos;
layout (location = 2) in uvec4 a_chunk_color;
layout (location = 3) in float a_offset;

uniform ivec2 render_size;
uniform int chunk_buf_diameter;
uniform mat4 mat_projection;
out vec3 vertex_position;
out vec4 vertex_color;
out float camera_distance;
vec2 ndc_scale = 2.0 / vec2(render_size);
float gizmo_scale = GIZMO_SCALE * (1.0 / chunk_buf_diameter) * ndc_scale.y;
float inv_255 = 1.0 / 255.0;

void main()
{
    /* a matrix to flatten layers and offset them in screen-space Z */
    mat4 mat_offset = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.001, 0.0,
            0.0, 0.0, a_offset, 1.0);

    vertex_position = a_pos * CUBE_SCALE + vec3(a_chunk_pos.xyz);
    vertex_color = a_chunk_color * inv_255;

    camera_distance = float(chunk_buf_diameter) / 2.0 + CAMERA_DISTANCE_PADDING;

    gl_Position =
        mat_offset *
        mat_projection *
        vec4(vertex_position * gizmo_scale, 1.0);
}
