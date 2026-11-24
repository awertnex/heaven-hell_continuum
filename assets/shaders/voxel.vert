#version 430 core

#define BLOCK_FACES_MAX 6

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00001111] 00; */
#define MASK_QUAD_POS           0x000000000000000f

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00000111] 00; */
#define MASK_QUAD_ID            0x0000000000000007

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000011 11111111] 00; */
#define MASK_QUAD_TEXTURE_ID    0x00000000000003ff

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00111111 11111111] 00; */
#define MASK_BLOCK_DATA         0x0000000000003fff

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000011 11111111] 00; */
#define MASK_BLOCK_ID           0x00000000000003ff

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 00001111] 00; */
#define MASK_BLOCK_LIGHT        0x000000000000000f

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000000 11110000] 00; */
#define MASK_BLOCK_AO           0x00000000000000f0

#define SHIFT_QUAD_X            0
#define SHIFT_QUAD_Y            4
#define SHIFT_QUAD_Z            8
#define SHIFT_QUAD_ID           12

#define SHIFT_QUAD_DATA         15
#define SHIFT_QUAD_TEXTURE_ID   15
#define SHIFT_QUAD_STATE        25

#define SHIFT_QUAD_LIGHT        0
#define SHIFT_QUAD_AO           4

layout (location = 0) in uint a_data;
layout (location = 1) in uint a_light;

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

vec3 normal_buf[BLOCK_FACES_MAX] = vec3[](
        vec3(1.0, 0.0, 0.0),
        vec3(-1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, -1.0, 0.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, -1.0));

const mat3 orientation_tab[BLOCK_FACES_MAX] = mat3[](
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
    uint quad_id = (a_data >> SHIFT_QUAD_ID) & MASK_QUAD_ID;
    uint block_id = (a_data >> SHIFT_QUAD_TEXTURE_ID) & MASK_BLOCK_ID;
    vec3 pos_voxel = block_world_offset + pos_fixup + vec3(
            float((a_data >> SHIFT_QUAD_X) & MASK_QUAD_POS),
            float((a_data >> SHIFT_QUAD_Y) & MASK_QUAD_POS),
            float((a_data >> SHIFT_QUAD_Z) & MASK_QUAD_POS));

    pos = vec4(pos_voxel + orientation_tab[quad_id] * pos_buf[gl_VertexID], 1.0);
    pos_view = mat_view * pos;
    uv = uv_buf[gl_VertexID];
    normal = normal_buf[quad_id];
    normal_view = mat_view_transpose * normal;

    texture_id = texture_buf[block_id * BLOCK_FACES_MAX + quad_id];
    block_light = float(a_light & MASK_BLOCK_LIGHT) / float(MASK_BLOCK_LIGHT);

    gl_Position = mat_perspective * pos;
}
