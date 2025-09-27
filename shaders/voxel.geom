#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = MAX_VERTICES) out;

#define MAX_VERTICES 36
/* 
 * from src/h/chunking.h */
enum BlockFlags
{
    POSITIVE_X =    0x0000000000010000,
    NEGATIVE_X =    0x0000000000020000,
    POSITIVE_Y =    0x0000000000040000,
    NEGATIVE_Y =    0x0000000000080000,
    POSITIVE_Z =    0x0000000000100000,
    NEGATIVE_Z =    0x0000000000200000,
    NOT_EMPTY =     0x0000000000400000,
    BLOCKFACES =    0x00000000007f0000,
    BLOCKID =       0x00000000000003ff,
    BLOCKSTATE =    0x0000000000007c00,
    BLOCKDATA =     0x0000000000007fff,
    BLOCKLIGHT =    0x000000001f000000,
    RLE_TRIGGER =   0x0000000000008000,
    BLOCK_X =       0x000000ff00000000,
    BLOCK_Y =       0x0000ff0000000000,
    BLOCK_Z =       0x00ff000000000000,
}; /* BlockFlags */

uniform vec3i mask_positive;
uniform vec3i mask_negative;
uniform vec3i mask_not_empty;
uniform vec3 chunk_position;
uniform mat mat_perspective;
in int voxel_data[];
out vec3 raw_color;

void main()
{
    vec3 base = vec3(
            (voxel_data[0] >> mask_positive.x) & 1),
            (voxel_data[0] >> mask_positive.y) & 1),
            (voxel_data[0] >> mask_positive.z) & 1)) + chunk_position;

    vec3 vbo_voxel[8] =
        vec3[](
                vec3(0.0, 0.0, 0.0),
                vec3(1.0, 0.0, 0.0),
                vec3(1.0, 1.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(1.0, 0.0, 1.0),
                vec3(1.0, 1.0, 1.0),
                vec3(0.0, 1.0, 1.0),
              );

    int ebo_voxel[MAX_VERTICES] =
        int[](
                0, 1, 2, 2, 3, 0,
                4, 5, 6, 6, 7, 4,
                0, 4, 7, 7, 3, 0,
                1, 5, 6, 6, 2, 1,
                3, 2, 6, 6, 7, 3,
                0, 1, 5, 5, 4, 0,
             );

    vec3 world_position = vec3(0.0);

    for (int i = 0; i < MAX_VERTICES; ++i)
    {
        world_position = base + vbo_voxel[ebo_voxel[i]];
        gl_Position = mat_projection * vec4(world_position, 1.0);
        EmitVertex();
        if ((i + 1) % 3 == 0) EndPrimitive();
    }
}

