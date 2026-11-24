#version 430 core

#define FACES_MAX 6

#define MASK_FACE_ID 0x00000007
#define MASK_BLOCK_ID 0x000003ff
#define MASK_BLOCK_LIGHT_VAL 0x0f
#define MASK_BLOCK_LIGHT_AO 0x0f

layout (location = 0) in vec3 a_quad_pos;
layout (location = 1) in vec3 a_quad_uv;
layout (location = 2) in uint a_data;
layout (location = 3) in uint a_light;

layout(std430, binding = 1) readonly buffer ssbo_texture_indices
{
    uint texture_buf[];
};

uniform mat4 mat_view;
uniform mat4 mat_perspective;
uniform vec3 block_world_offset;
out vec4 pos;
out vec4 pos_view;
out vec2 uv;
out vec3 normal;
out vec3 normal_view;
out flat uint texture_id;
out float block_light;
out float ao_weights[4];

const vec3 pos_buf[4] = vec3[](
        vec3(0.5, -0.5, -0.5),
        vec3(0.5, -0.5, 0.5),
        vec3(0.5, 0.5, 0.5),
        vec3(0.5, 0.5, -0.5));
vec3 pos_fixup = vec3(0.5, 0.5, 0.5);

const vec2 uv_buf[4] = vec2[](
        vec2(0.0, 1.0),
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(1.0, 1.0));

vec3 normal_buf[FACES_MAX] = vec3[](
        vec3(1.0, 0.0, 0.0),
        vec3(-1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, -1.0, 0.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, -1.0));

const mat3 orientation_tab[FACES_MAX] = mat3[](
        mat3( /* px */
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0),
        mat3( /* nx */
            -1.0, 0.0, 0.0,
            0.0, -1.0, 0.0,
            0.0, 0.0, 1.0),
        mat3( /* py */
            0.0, 1.0, 0.0,
            -1.0, 0.0, 0.0,
            0.0, 0.0, 1.0),
        mat3( /* ny */
            0.0, -1.0, 0.0,
            1.0, 0.0, 0.0,
            0.0, 0.0, 1.0),
        mat3( /* pz */
            0.0, 0.0, 1.0,
            0.0, 1.0, 0.0,
            -1.0, 0.0, 0.0),
        mat3( /* nz */
            0.0, 0.0, -1.0,
            0.0, 1.0, 0.0,
            1.0, 0.0, 0.0));

void main()
{
    mat3 mat_view_transpose = transpose(inverse(mat3(mat_view)));
    uint face_id = (a_data >> 4) & MASK_FACE_ID;
    uint block_id = (a_data >> 19) & MASK_BLOCK_ID;
    vec3 pos_voxel = block_world_offset + pos_fixup + vec3(
            float((a_data >> 15) & 0xf),
            float((a_data >> 11) & 0xf),
            float((a_data >> 7) & 0xf));

    pos = vec4(pos_voxel + orientation_tab[face_id] * pos_buf[gl_VertexID], 1.0);
    pos_view = mat_view * pos;
    uv = uv_buf[gl_VertexID];
    normal = normal_buf[face_id];
    normal_view = mat_view_transpose * normal;

    texture_id = texture_buf[block_id * FACES_MAX + face_id];
    block_light = float(a_light & MASK_BLOCK_LIGHT_VAL) / float(MASK_BLOCK_LIGHT_VAL);

    gl_Position = mat_perspective * pos;
}
