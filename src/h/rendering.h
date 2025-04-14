#pragma once

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"
#include "../engine/h/defines.h"

// ---- definitions ------------------------------------------------------------
typedef struct Vertex
{
    Vector3 pos;
    Vector2 texCoord;
} Vertex;

typedef struct RenderData
{
    Vector3 vertices;
    Vector2 textureCoordinates;
} RenderData;

// ---- declarations -----------------------------------------------------------
extern u32 VAO, VBO, EBO;
extern u32 textureDirt;

// ---- signatures -------------------------------------------------------------
void init_rendering();
void update_rendering();
void draw_rendering();
void free_rendering();
