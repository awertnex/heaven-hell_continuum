#version 430 core

#define FACE_VERTICES   6
#define MAX_VERTICES    36

layout(points) in;
layout(triangle_strip, max_vertices = MAX_VERTICES) out;

/* from src/h/chunking.h */
#define POSITIVE_X      0x0000000000010000
#define NEGATIVE_X      0x0000000000020000
#define POSITIVE_Y      0x0000000000040000
#define NEGATIVE_Y      0x0000000000080000
#define POSITIVE_Z      0x0000000000100000
#define NEGATIVE_Z      0x0000000000200000
#define NOT_EMPTY       0x0000000000400000

in int vs_data[];
in vec3 vs_position[];
out vec4 gs_diffuse;
out vec3 gs_position;
uniform mat4 mat_perspective;

void main()
{
    vec3 vbo[8] =
        vec3[](
                vec3(0.0, 0.0, 0.0),
                vec3(1.0, 0.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(1.0, 1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(1.0, 0.0, 1.0),
                vec3(0.0, 1.0, 1.0),
                vec3(1.0, 1.0, 1.0));

    int ebo[MAX_VERTICES] =
        int[](
                1, 5, 7, 7, 3, 1,
                2, 6, 4, 4, 0, 2,
                3, 7, 6, 6, 2, 3,
                0, 4, 5, 5, 1, 0,
                4, 6, 7, 7, 5, 4,
                0, 1, 3, 3, 2, 0);

    if (bool(vs_data[0] & NOT_EMPTY))
    {
        if (bool(vs_data[0] & POSITIVE_X))
            for (int i = 0; i < FACE_VERTICES; ++i)
            {
                gs_diffuse = vec4(0.9, 0.9, 0.9, 1.0);
                gs_position = vs_position[0] +
                    vbo[ebo[i]];

                gl_Position = mat_perspective * vec4(gs_position, 1.0);
                EmitVertex();
                if ((i + 1) % 3 == 0) EndPrimitive();
            }

        if (bool(vs_data[0] & NEGATIVE_X))
            for (int i = 0; i < FACE_VERTICES; ++i)
            {
                gs_diffuse = vec4(0.7, 0.7, 0.7, 1.0);
                gs_position = vs_position[0] +
                    vbo[ebo[i + FACE_VERTICES]];

                gl_Position = mat_perspective * vec4(gs_position, 1.0);
                EmitVertex();
                if ((i + 1) % 3 == 0) EndPrimitive();
            }

        if (bool(vs_data[0] & POSITIVE_Y))
            for (int i = 0; i < FACE_VERTICES; ++i)
            {
                gs_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
                gs_position = vs_position[0] +
                    vbo[ebo[i + (FACE_VERTICES * 2)]];

                gl_Position = mat_perspective * vec4(gs_position, 1.0);
                EmitVertex();
                if ((i + 1) % 3 == 0) EndPrimitive();
            }

        if (bool(vs_data[0] & NEGATIVE_Y))
            for (int i = 0; i < FACE_VERTICES; ++i)
            {
                gs_diffuse = vec4(0.6, 0.6, 0.6, 1.0);
                gs_position = vs_position[0] +
                    vbo[ebo[i + (FACE_VERTICES * 3)]];

                gl_Position = mat_perspective * vec4(gs_position, 1.0);
                EmitVertex();
                if ((i + 1) % 3 == 0) EndPrimitive();
            }

        if (bool(vs_data[0] & POSITIVE_Z))
            for (int i = 0; i < FACE_VERTICES; ++i)
            {
                gs_diffuse = vec4(1.0, 1.0, 1.0, 1.0);
                gs_position = vs_position[0] +
                    vbo[ebo[i + (FACE_VERTICES * 4)]];

                gl_Position = mat_perspective * vec4(gs_position, 1.0);
                EmitVertex();
                if ((i + 1) % 3 == 0) EndPrimitive();
            }

        if (bool(vs_data[0] & NEGATIVE_Z))
            for (int i = 0; i < FACE_VERTICES; ++i)
            {
                gs_diffuse = vec4(0.3, 0.3, 0.3, 1.0);
                gs_position = vs_position[0] +
                    vbo[ebo[i + (FACE_VERTICES * 5)]];

                gl_Position = mat_perspective * vec4(gs_position, 1.0);
                EmitVertex();
                if ((i + 1) % 3 == 0) EndPrimitive();
            }
    }
}
