#ifndef MC_C_RENDERING_H

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"
#include "../engine/h/defines.h"

// ---- definitions ------------------------------------------------------------
typedef struct Vertex
{
    Vector3 pos;
    Vector2 tex_coord;
} Vertex;

typedef struct RenderData
{
    Vector3 vertices;
    Vector2 tex_coord;
} RenderData;

// ---- declarations -----------------------------------------------------------
extern u32 vao, vbo, ebo;
extern u32 texture_dirt;

// ---- signatures -------------------------------------------------------------
void init_rendering();
void update_rendering();
void draw_rendering();
void free_rendering();

#define MC_C_RENDERING_H
#endif

