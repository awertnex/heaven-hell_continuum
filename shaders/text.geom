#version 330 core

#define MAX_VERTICES 6
layout (points) in;
layout (triangle_strip, max_vertices = MAX_VERTICES) out;

in vec2 vs_tex_coords[];
out vec2 tex_coords;
uniform float char_size;
uniform vec2 font_size;
uniform vec2 screen_size;

void main()
{
    vec4 vbo_quad[MAX_VERTICES] =
        vec4[](
                /* vertex_pos, tex_coords */
                vec4(0.0, -2.0, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 0.0),
                vec4(2.0, 0.0, 1.0, 0.0),
                vec4(2.0, 0.0, 1.0, 0.0),
                vec4(2.0, -2.0, 1.0, 1.0),
                vec4(0.0, -2.0, 0.0, 1.0)
              );

    mat4 projection =
        mat4(
                font_size.x, 0.0, 0.0, 0.0,
                0.0, font_size.y, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                -1.0, 1.0, 0.0, 1.0);

    for (int i = 0; i < MAX_VERTICES; ++i)
    {
        gl_Position = projection * (gl_in[0].gl_Position +
             vec4(vbo_quad[i].xy, 0.0, 0.0));
        tex_coords = vs_tex_coords[0] +
            (vbo_quad[i].zw * char_size);
        EmitVertex();
        if ((i + 1) % 3 == 0)
            EndPrimitive();
    }
}

