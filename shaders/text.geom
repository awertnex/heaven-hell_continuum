#version 330 core

#define MAX_VERTICES 4
layout (points) in;
layout (triangle_strip, max_vertices = MAX_VERTICES) out;

in vec2 texture_sample[];
out vec2 tex_coords;
uniform float char_size;
uniform vec2 font_size;
uniform vec2 ndc_size;
uniform vec2 offset;
uniform float advance;
uniform float bearing;

void main()
{
    vec2 vbo_quad[8] =
        vec2[](
                /* vertex position */
                vec2(0.0, 0.0),
                vec2(0.0, 1.0),
                vec2(1.0, 1.0),
                vec2(1.0, 0.0),
 
                /* texture coordinates */
                vec2(0.0, 1.0),
                vec2(0.0, 0.0),
                vec2(1.0, 0.0),
                vec2(1.0, 1.0)
              );
 
    mat4 projection =
        mat4(
                font_size.x, 0.0, 0.0, 0.0,
                0.0, font_size.y, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                offset.x + (advance * ndc_size.x),
                offset.y + (bearing * ndc_size.y),
                0.0, 1.0
            );

    for (int i = 0; i < MAX_VERTICES; ++i)
    {
        gl_Position = projection *
            (gl_in[0].gl_Position + vec4(vbo_quad[i], 0.0, 0.0));
        tex_coords = vec2(
                (texture_sample[i].s + vbo_quad[i + 4].s) * char_size,
                (texture_sample[i].t + vbo_quad[i + 4].t) * char_size);
        EmitVertex();
    }
    EndPrimitive();
}

