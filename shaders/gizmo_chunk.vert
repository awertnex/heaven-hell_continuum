#version 430 core

#define MARGIN 200.0
layout (location = 0) in vec3 a_pos;

uniform ivec2 render_size;
uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;
uniform vec3 cursor;
uniform float size;
out vec3 vertex_position;

void main()
{
    float gizmo_scale = 0.006;

    mat4 mat_offset = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            (render_size.x - MARGIN) / render_size.x,
            (render_size.y - MARGIN) / render_size.y,
            0.0, 1.0);

    vec3 voxel_size = (a_pos * size) + ((1.0 - size) / 2.0);
    vertex_position = voxel_size + cursor;
    gl_Position =
        mat_offset *
        mat_projection *
        mat_orientation *
        mat_rotation *
        mat_translation *
        vec4(vertex_position * gizmo_scale, 1.0);
}
