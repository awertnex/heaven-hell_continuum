#version 330 core

#ifdef GL_ES
precision lowp float;
#endif // GL_ES

// ---- declarations -----------------------------------------------------------
uniform vec2 cursor_pos;
in vec2 render_size_delta;
in float render_ratio;
in vec4 vertex_color;
float distance;
out vec4 FragColor;

// ---- signatures -------------------------------------------------------------
float square(float n);
float get_distance(vec4 a, vec4 b);

void main()
{
    float sharpness = 2.0;
    vec2 cursor_pos_delta = vec2(
            (cursor_pos.x - 0.5) * (render_ratio * 2.0),
            (cursor_pos.y - 0.5) * 2.5);
    distance = ((get_distance(vertex_color, vec4(cursor_pos_delta, 0.0, 1.0)) - 1.0) * -1.0);

    FragColor = vec4(
            distance,
            distance - 0.4,
            distance - 0.6,
            1.0);
}

// ---- functions --------------------------------------------------------------
float square(float n)
{
    return ((n) * (n));
}

float get_distance(vec4 a, vec4 b)
{
    return (square(a.x - b.x) + square(a.y - b.y) + square(a.z - b.z));
}

