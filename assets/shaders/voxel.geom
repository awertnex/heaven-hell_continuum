#version 430 core

#define FACE_VERTICES 6
#define VERTICES_MAX 36

layout(points) in;
layout(triangle_strip, max_vertices = VERTICES_MAX) out;

#define FLAG_POSITIVE_X 0x00010000
#define FLAG_NEGATIVE_X 0x00020000
#define FLAG_POSITIVE_Y 0x00040000
#define FLAG_NEGATIVE_Y 0x00080000
#define FLAG_POSITIVE_Z 0x00100000
#define FLAG_NEGATIVE_Z 0x00200000

void voxel_make()
{
    int i = 0;
    int j = 0;

    vec3 vertex_buf[8] =
        vec3[](
                vec3(0.0, 0.0, 0.0),
                vec3(1.0, 0.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(1.0, 1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(1.0, 0.0, 1.0),
                vec3(0.0, 1.0, 1.0),
                vec3(1.0, 1.0, 1.0));

    vec3 normal_buf[6] =
        vec3[](
                vec3(1.0, 0.0, 0.0),
                vec3(-1.0, 0.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(0.0, -1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(0.0, 0.0, -1.0));

    int index_buf[VERTICES_MAX] =
        int[](
                1, 5, 7, 7, 3, 1,
                2, 6, 4, 4, 0, 2,
                3, 7, 6, 6, 2, 3,
                0, 4, 5, 5, 1, 0,
                4, 6, 7, 7, 5, 4,
                0, 1, 3, 3, 2, 0);

    vec2 gs_uv[4] =
        vec2[](
                vec2(0.0, 1.0),
                vec2(0.0, 0.0),
                vec2(1.0, 0.0),
                vec2(1.0, 1.0));

    int index_buf_uv[FACE_VERTICES] =
        int[](0, 1, 2, 2, 3, 0);

    mat3 mat_view_transpose = transpose(inverse(mat3(mat_view)));

    for (i = 0; i < FACES_MAX; ++i)
    {
        if (bool(vs_block_faces[0] & (1 << i)))
            for (j = 0; j < FACE_VERTICES; ++j)
            {
                pos = vec4(vs_pos[0] + vertex_buf[index_buf[j + FACE_VERTICES * i]], 1.0);
                pos_view = mat_view * pos;
                uv = gs_uv[index_buf_uv[j]];
                face_index = texture_buf[vs_block_id[0] * 6 + i];
                normal = normal_buf[i];
                normal_view = mat_view_transpose * normal;

                gl_Position = mat_perspective * pos;
                EmitVertex();
                if ((j + 1) % 3 == 0) EndPrimitive();
            }
    }
}

void main()
{
    voxel_make();
}
