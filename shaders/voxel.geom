#version 430 core

#define MAX_VERTICES 36

layout(points) in;
layout(triangle_strip, max_vertices = MAX_VERTICES) out;

/* 
 * from src/h/chunking.h */
#define POSITIVE_X      0x0000000000010000
#define NEGATIVE_X      0x0000000000020000
#define POSITIVE_Y      0x0000000000040000
#define NEGATIVE_Y      0x0000000000080000
#define POSITIVE_Z      0x0000000000100000
#define NEGATIVE_Z      0x0000000000200000
#define NOT_EMPTY       0x0000000000400000

in int vs_data[];
out vec3 vertex_position;
uniform vec3 chunk_position;
uniform mat4 mat_perspective;

void main()
{
    vec3 base = chunk_position;

    vec3 vbo_voxel[8] =
        vec3[](
                vec3(0.0, 0.0, 0.0),
                vec3(1.0, 0.0, 0.0),
                vec3(1.0, 1.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(1.0, 0.0, 1.0),
                vec3(1.0, 1.0, 1.0),
                vec3(0.0, 1.0, 1.0)
              );

    int ebo_voxel[MAX_VERTICES] =
        int[](
                0, 1, 2, 2, 3, 0,
                4, 5, 6, 6, 7, 4,
                0, 4, 7, 7, 3, 0,
                1, 5, 6, 6, 2, 1,
                3, 2, 6, 6, 7, 3,
                0, 1, 5, 5, 4, 0
             );

    vec3 world_position = vec3(0.0);

    if (voxel_data[0] & NOT_EMPTY)
    {
        if (voxel_data[0] & POSITIVE_X)
            for (int i = 0; i < MAX_VERTICES; ++i)
            {
                world_position = base + vbo_voxel[ebo_voxel[i]];
                gl_Position = mat_perspective * vec4(world_position, 1.0);
                vertex_position = gl_Position.xyz;
                EmitVertex();
                if ((i + 1) % 3 = 0) EndPrimitive();
            }

    }
}

