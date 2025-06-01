#version 330 core

layout (location = 0) in vec3 vertex_pos;

// ---- declarations -----------------------------------------------------------
uniform vec2 render_size;
uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_projection;
out vec2 render_size_delta;
out float render_ratio;
out vec4 vertex_color;

void main()
{
    render_size_delta = render_size;
    render_ratio = (render_size.x / render_size.y);

    vertex_color = vec4(vertex_pos, 1.0);

    gl_Position =
        mat_projection
        * mat_view
        * mat_model
        * vec4(vertex_pos, 1.0);
}
