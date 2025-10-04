#version 430 core

layout (location = 0) in int a_data;
layout (location = 1) in int a_pos;

/* from src/h/chunking.h */
#define BLOCK_X(i) float((i >> 32) & 0xf)
#define BLOCK_Y(i) float((i >> 36) & 0xf)
#define BLOCK_Z(i) float((i >> 40) & 0xf)

out int vs_data;
out vec3 vs_position;
uniform vec3 chunk_position;

void main()
{
    vs_data = a_data;
    vs_position = chunk_position + vec3(
            BLOCK_X(a_pos),
            BLOCK_Y(a_pos),
            BLOCK_Z(a_pos));
}
