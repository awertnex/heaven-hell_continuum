#version 430 core

#define MASK_BLOCK_ID 0x000003ff
#define MASK_BLOCK_LIGHT_VAL 0x0f
#define MASK_BLOCK_LIGHT_AO 0xf0

layout (location = 0) in uint a_data;
layout (location = 1) in uint a_pos;
layout (location = 2) in uint a_light;

uniform vec3 block_world_offset;
out uint vs_block_id;
out uint vs_block_faces;
out vec3 vs_pos;
out float vs_light;
out float vs_ao_weights[4];

void main()
{
    vs_block_id = a_data & MASK_BLOCK_ID;
    vs_block_faces = a_data >> 16;
    vs_pos = block_world_offset + vec3(
            (a_pos >> 0) & 0xf,
            (a_pos >> 4) & 0xf,
            (a_pos >> 8) & 0xf);
    vs_light = float(a_light & MASK_BLOCK_LIGHT_VAL) / float(MASK_BLOCK_LIGHT_VAL);
}
