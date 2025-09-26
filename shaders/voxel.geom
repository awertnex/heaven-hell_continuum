#version 330 core

#define MAX_VERTICES 36

layout(points) in;
layout(triangle_strip, max_vertices = MAX_VERTICES) out;

uniform mat mat_perspective;
uniform vec3 chunk_position;
in vec3 voxel_data[];
out vec3 raw_color;

void main()
{
    vec3 base = voxel_data[0] + chunk_position;

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
        orld_position = base + vbo_voxel[ebo_voxel[i]];
        gl_Position = mat_projection * vec4(world_position, 1.0);
        EmitVertex();
        if ((i + 1) % 3 == 0) EndPrimitive();
    }
}

