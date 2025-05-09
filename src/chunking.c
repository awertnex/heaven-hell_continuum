#include <math.h>

#include "h/chunking.h"
#include "h/logic.h"

#include "engine/h/memory.h"
#include "engine/h/logger.h"

u16 world_height = WORLD_HEIGHT_NORMAL;
Chunk *chunk_buf = {0};                         // chunk buffer, raw chunk data
Chunk *chunk_tab[CHUNK_BUF_ELEMENTS] = {NULL};  // chunk pointer look-up table
Chunk *chunk_reg[CHUNK_BUF_DIAMETER] = {NULL};  // register for chunks on edge of render distance facing one direction
v2u16 chunk_tab_coordinates;                    // pointer arithmetic redundancy optimization
Chunk *target_chunk; //temp
struct WorldStats world_stats =
{
    .block_count = 0,
    .quad_count = 0,
};
u8 opacity = 0;

u8 init_chunking()
{
    MC_C_ALLOC(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));
    return 0;

cleanup:
    free_chunking();
    return -1;
}

void free_chunking()
{
    MC_C_FREE(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));
}

//TODO: check chunk-border block-faces
void add_block(Chunk *chunk, u8 x, u8 y, u16 z)
{
    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;

    if (x < CHUNK_DIAMETER - 1)
        chunk->i[z][y][x + 1] ? (chunk->i[z][y][x + 1] &= ~NEGATIVE_X) : (chunk->i[z][y][x] |= POSITIVE_X);
    else chunk->i[z][y][x] |= POSITIVE_X;

    if (x > 0)
        chunk->i[z][y][x - 1] ? (chunk->i[z][y][x - 1] &= ~POSITIVE_X) : (chunk->i[z][y][x] |= NEGATIVE_X);
    else chunk->i[z][y][x] |= NEGATIVE_X;

    if (y < CHUNK_DIAMETER - 1)
        chunk->i[z][y + 1][x] ? (chunk->i[z][y + 1][x] &= ~NEGATIVE_Y) : (chunk->i[z][y][x] |= POSITIVE_Y);
    else chunk->i[z][y][x] |= POSITIVE_Y;

    if (y > 0)
        chunk->i[z][y - 1][x] ? (chunk->i[z][y - 1][x] &= ~POSITIVE_Y) : (chunk->i[z][y][x] |= NEGATIVE_Y);
    else chunk->i[z][y][x] |= NEGATIVE_Y;

    if (z < world_height - 1)
        chunk->i[z + 1][y][x] ? (chunk->i[z + 1][y][x] &= ~NEGATIVE_Z) : (chunk->i[z][y][x] |= POSITIVE_Z);
    else chunk->i[z][y][x] |= POSITIVE_Z;
    
    if (z > 0)
        chunk->i[z - 1][y][x] ? (chunk->i[z - 1][y][x] &= ~POSITIVE_Z) : (chunk->i[z][y][x] |= NEGATIVE_Z);
    else chunk->i[z][y][x] |= NEGATIVE_Z;

    chunk->i[z][y][x] |= NOT_EMPTY;

    if (GET_BLOCK_INDEX(x, y, z) >= chunk->block_parse_limit)
        chunk->block_parse_limit = GET_BLOCK_INDEX(x, y, z);
}

//TODO: check chunk barrier faces
void remove_block(Chunk *chunk, u8 x, u8 y, u16 z)
{
    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;

    if (x < CHUNK_DIAMETER - 1)
        chunk->i[z][y][x + 1] ? (chunk->i[z][y][x + 1] |= NEGATIVE_X) : 0;

    if (x > 0)
        chunk->i[z][y][x - 1] ? (chunk->i[z][y][x - 1] |= POSITIVE_X) : 0;

    if (y < CHUNK_DIAMETER - 1)
        chunk->i[z][y + 1][x] ? (chunk->i[z][y + 1][x] |= NEGATIVE_Y) : 0;

    if (y > 0)
        chunk->i[z][y - 1][x] ? (chunk->i[z][y - 1][x] |= POSITIVE_Y) : 0;

    if (z < world_height - 1)
        chunk->i[z + 1][y][x] ? (chunk->i[z + 1][y][x] |= NEGATIVE_Z) : 0;
    
    if (z > 0)
        chunk->i[z - 1][y][x] ? (chunk->i[z - 1][y][x] |= POSITIVE_Z) : 0;

    chunk->i[z][y][x] = 0;
}

void generate_chunk(Chunk *chunk) // TODO: make this function
{
}

void unload_chunk(Chunk *chunk) // TODO: make this function
{
}

Chunk *push_chunk_buf(v2i16 *player_chunk, v2u16 chunk_tab_coordinates)
{
    u32 i = 0; u16 z = 0; u8 y = 0, x = 0;
    for (; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (chunk_buf[i].state & STATE_CHUNK_LOADED) continue;

        memset(&chunk_buf[i], 0, sizeof(Chunk));
        chunk_buf[i] =
            (Chunk){
                .pos = (v2i16){
                    player_chunk->x + (chunk_tab_coordinates.x - CHUNK_BUF_RADIUS),
                    player_chunk->y + (chunk_tab_coordinates.y - CHUNK_BUF_RADIUS)},
                .mat = LoadMaterialDefault(),
                .state = 0 | STATE_CHUNK_LOADED | STATE_CHUNK_RENDER,
            };
        chunk_buf[i].id = ((chunk_buf[i].pos.x & 0xffff) << 16) + (chunk_buf[i].pos.y & 0xffff);
        for (; z < 3; ++z)
            for (y = 0; y < CHUNK_DIAMETER; ++y)
                for (x = 0; x < CHUNK_DIAMETER; ++x)
                    add_block(&chunk_buf[i], x, y, z);

        return &chunk_buf[i];
    }
    return NULL;
}

void update_chunk_buf(v2i16 *player_chunk)
{
    for (u32 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        chunk_tab_coordinates =
            (v2u16){
                i % CHUNK_BUF_DIAMETER,
                (i32)floorf((f32)i / CHUNK_BUF_DIAMETER)
            };

        if (is_distance_within(setting.render_distance,
                    (v2i32){CHUNK_BUF_RADIUS, CHUNK_BUF_RADIUS},
                    (v2i32){chunk_tab_coordinates.x, chunk_tab_coordinates.y}))
        {
            if (chunk_tab[i] == NULL)
            {
                chunk_tab[i] =
                    push_chunk_buf(player_chunk,
                            chunk_tab_coordinates);

                if (chunk_tab[i] != NULL)
                    LOGINFO("Chunk Loaded 'id[%08x] pos[%03d %03d] i[%03d]'",
                            chunk_tab[i]->id,
                            chunk_tab[i]->pos.x,
                            chunk_tab[i]->pos.y,
                            i);
            }
        }
        else if (chunk_tab[i] != NULL) // TODO: implement chunk unloading elsewhere
            if (chunk_tab[i]->state & STATE_CHUNK_LOADED)
            {
                memset(chunk_tab[i], 0, sizeof(Chunk));
                chunk_tab[i] = NULL;
            }
    }
}

Chunk *get_chunk(v3i32 *coordinates, u16 *state, u16 flag) // TODO: revise, might not be needed
{
    v2i32 coordinates_parsed =
    {
        floorf((f32)coordinates->x / CHUNK_DIAMETER),
        floorf((f32)coordinates->y / CHUNK_DIAMETER),
    };

    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
        if (chunk_tab[i] != NULL)
            if (chunk_tab[i]->pos.x == coordinates_parsed.x &&
                    chunk_tab[i]->pos.y == coordinates_parsed.y)
            {
                *state |= flag;
                return chunk_tab[i];
            }
    *state &= ~flag;
    return NULL;
}

Texture2D tex_cobblestone; //temp texturing
Texture2D tex_dirt; //temp texturing
void draw_chunk_tab()
{
    if (state & STATE_DEBUG_MORE)
        opacity = 200;
    else
        opacity = 255;

    rlPushMatrix();
    rlSetTexture(tex_cobblestone.id); //temp texturing
    rlBegin(RL_QUADS);

    v2f32 chunk_pos = {0};
    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
    {
        if (chunk_tab[i] == NULL) continue;
        if (!(chunk_tab[i]->state & STATE_CHUNK_RENDER)) continue;

        chunk_pos =
            (v2f32){
                chunk_tab[i]->pos.x * CHUNK_DIAMETER,
                chunk_tab[i]->pos.y * CHUNK_DIAMETER};

        rlTranslatef(chunk_pos.x, chunk_pos.y, WORLD_BOTTOM);

        for (u32 j = 0; j <= chunk_tab[i]->block_parse_limit; ++j)
        {
            if (chunk_tab[i]->i[GET_BLOCK_Z(j)][GET_BLOCK_Y(j)][GET_BLOCK_X(j)] & BLOCKFACES)
                draw_block(chunk_tab[i]->i[GET_BLOCK_Z(j)][GET_BLOCK_Y(j)][GET_BLOCK_X(j)]);
            rlTranslatef(1.0f, 0.0f, 0.0f);

            if (GET_BLOCK_X(j) == (CHUNK_DIAMETER - 1))
            {
                rlTranslatef(-CHUNK_DIAMETER, 1.0f, 0.0f);
                if (GET_BLOCK_Y(j) == (CHUNK_DIAMETER - 1))
                    rlTranslatef(0.0f, -CHUNK_DIAMETER, 1.0f);
            }
        }

        rlTranslatef(
                -GET_BLOCK_X(chunk_tab[i]->block_parse_limit + 1) - (chunk_pos.x),
                -GET_BLOCK_Y(chunk_tab[i]->block_parse_limit + 1) - (chunk_pos.y),
                -GET_BLOCK_Z(chunk_tab[i]->block_parse_limit + 1) - WORLD_BOTTOM);
    }

    rlEnd();
    rlPopMatrix();
    rlSetTexture(0); //temp texturing
}

// raylib/rmodels.c/DrawCube refactored
void draw_block(u32 block)
{
    if (block & POSITIVE_X)
    {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(150, 150, 137, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(200, 210, 90, opacity);
        
        rlNormal3f(1.0f, 0.0f, 0.0f); //temp texturing
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
    }

    if (block & NEGATIVE_X)
    {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(135, 135, 123, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(236, 17, 90, opacity);

        rlNormal3f(-1.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
    }

    if (block & POSITIVE_Y)
    {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(155, 155, 142, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(200, 248, 246, opacity);

        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
    }

    if (block & NEGATIVE_Y)
    {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(140, 140, 123, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(28, 14, 50, opacity);

        rlNormal3f(0.0f, -1.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
    }

    if (block & POSITIVE_Z)
    {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(176, 176, 160, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(250, 18, 5, opacity);

        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
    }

    if (block & NEGATIVE_Z)
    {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(115, 115, 104, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(200, 40, 203, opacity);

        rlNormal3f(0.0f, 0.0f, -1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
    }
}

// raylib/rmodels.c/DrawLine3D refactored
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, Color color)
{
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(pos_0.x, pos_0.y, pos_0.z);
    rlVertex3f(pos_1.x, pos_1.y, pos_1.z);
}

// raylib/rmodels.c/DrawCubeWires refactored
void draw_block_wires(v3i32 pos)
{
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlBegin(RL_LINES);
    rlColor4ub(0, 0, 0, 150);

    rlVertex3f(0.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 0.0f, 1.0f);

    rlVertex3f(1.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 1.0f, 1.0f);

    rlVertex3f(1.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 1.0f, 1.0f);

    rlVertex3f(0.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 1.0f);

    rlVertex3f(0.0f, 0.0f, 0.0f);
    rlVertex3f(1.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 0.0f, 0.0f);
    rlVertex3f(1.0f, 1.0f, 0.0f);

    rlVertex3f(1.0f, 1.0f, 0.0f);
    rlVertex3f(0.0f, 1.0f, 0.0f);

    rlVertex3f(0.0f, 1.0f, 0.0f);
    rlVertex3f(0.0f, 0.0f, 0.0f);

    rlVertex3f(0.0f, 0.0f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 1.0f, 1.0f);
    rlVertex3f(1.0f, 1.0f, 0.0f);

    rlVertex3f(0.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 1.0f, 0.0f);

    rlEnd();
    rlPopMatrix();
}

// raylib/rmodels.c/DrawCubeWires refactored
void draw_bounding_box(Vector3 origin, Vector3 scl)
{
    rlPushMatrix();
    rlTranslatef(
            origin.x - (scl.x / 2),
            origin.y - (scl.y / 2),
            origin.z);
    rlBegin(RL_LINES);
    rlColor4ub(255, 200, 220, 255);

    rlVertex3f(0.0f,    0.0f,   scl.z);
    rlVertex3f(scl.x,   0.0f,   scl.z);

    rlVertex3f(scl.x,   0.0f,   scl.z);
    rlVertex3f(scl.x,   scl.y,  scl.z);

    rlVertex3f(scl.x,   scl.y,  scl.z);
    rlVertex3f(0.0f,    scl.y,  scl.z);

    rlVertex3f(0.0f,    scl.y,  scl.z);
    rlVertex3f(0.0f,    0.0f,   scl.z);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(scl.x,   0.0f,   0.0f);

    rlVertex3f(scl.x,   0.0f,   0.0f);
    rlVertex3f(scl.x,   scl.y,  0.0f);

    rlVertex3f(scl.x,   scl.y,  0.0f);
    rlVertex3f(0.0f,    scl.y,  0.0f);

    rlVertex3f(0.0f,    scl.y,  0.0f);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(0.0f,    0.0f,   scl.z);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(scl.x,   0.0f,   scl.z);
    rlVertex3f(scl.x,   0.0f,   0.0f);

    rlVertex3f(scl.x,   scl.y,  scl.z);
    rlVertex3f(scl.x,   scl.y,  0.0f);

    rlVertex3f(0.0f,    scl.y,  scl.z);
    rlVertex3f(0.0f,    scl.y,  0.0f);

    rlEnd();
    rlPopMatrix();
}

void draw_bounding_box_clamped(Vector3 origin, Vector3 scl)
{
    Vector3 start = (Vector3){
            floorf(origin.x - 2.0f),
            floorf(origin.y - 2.0f),
            floorf(origin.z - 1.0f),
        };

    Vector3 end = (Vector3){
            ceilf(scl.x + 4.0f),
            ceilf(scl.y + 4.0f),
            ceilf(scl.z + 3.0f),
        };

    rlPushMatrix();
    rlTranslatef(start.x, start.y, start.z);
    rlBegin(RL_LINES);
    rlColor4ub(5, 209, 255, 255);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(0.0f,    0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  end.z);
    rlVertex3f(0.0f,    0.0f,   end.z);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(0.0f,    0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  end.z);

    rlEnd();
    rlPopMatrix();
}

