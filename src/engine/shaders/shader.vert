#version 330 core

layout (location = 0) in vec3 vertex_pos;

// ---- declarations -----------------------------------------------------------
uniform vec2 render_size;
out vec2 render_size_delta;
out float render_ratio;
out vec4 vertex_color;

void main()
{
    render_size_delta = render_size;
    render_ratio = (render_size.x / render_size.y);

    gl_Position = vec4(
            vertex_pos.x - 1.0,
            vertex_pos.y - 1.0,
            vertex_pos.z - 1.0,
            1.0);

    vertex_color = vec4(
            vertex_pos.x,
            vertex_pos.y * -1,
            vertex_pos.z, 1.0);
}
