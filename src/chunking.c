#include <stdio.h>
#include <math.h>

#include "h/chunking.h"
#include "h/logic.h"
#include "h/logger.h"

u16 worldHeight = WORLD_HEIGHT_NORMAL;
Chunk chunkBuf[(SETTING_RENDER_DISTANCE_MAX*2) + 1][(SETTING_RENDER_DISTANCE_MAX*2) + 1] = {0};
void *chunk_table[(SETTING_RENDER_DISTANCE_MAX*2) + 1][(SETTING_RENDER_DISTANCE_MAX*2) + 1] = {0};
Chunk *targetChunk = 0;
u64 blockCount = 0; //debug mode
u64 quadCount = 0; //debug mode
u8 opacity = 0;

void init_chunking()
{
    //TODO: write something in 'init_chunking()'
}

//TODO: check chunk barrier faces
void add_block(Chunk *chunk, u8 x, u8 y, u16 z)
{
    x %= CHUNK_SIZE;
    y %= CHUNK_SIZE;

    if (x < CHUNK_SIZE - 1)
        chunk->i[z][y][x + 1] ? (chunk->i[z][y][x + 1] &= ~NEGATIVE_X) : (chunk->i[z][y][x] |= POSITIVE_X);
    else chunk->i[z][y][x] |= POSITIVE_X;

    if (x > 0)
        chunk->i[z][y][x - 1] ? (chunk->i[z][y][x - 1] &= ~POSITIVE_X) : (chunk->i[z][y][x] |= NEGATIVE_X);
    else chunk->i[z][y][x] |= NEGATIVE_X;

    if (y < CHUNK_SIZE - 1)
        chunk->i[z][y + 1][x] ? (chunk->i[z][y + 1][x] &= ~NEGATIVE_Y) : (chunk->i[z][y][x] |= POSITIVE_Y);
    else chunk->i[z][y][x] |= POSITIVE_Y;

    if (y > 0)
        chunk->i[z][y - 1][x] ? (chunk->i[z][y - 1][x] &= ~POSITIVE_Y) : (chunk->i[z][y][x] |= NEGATIVE_Y);
    else chunk->i[z][y][x] |= NEGATIVE_Y;

    if (z < worldHeight - 1)
        chunk->i[z + 1][y][x] ? (chunk->i[z + 1][y][x] &= ~NEGATIVE_Z) : (chunk->i[z][y][x] |= POSITIVE_Z);
    else chunk->i[z][y][x] |= POSITIVE_Z;
    
    if (z > 0)
        chunk->i[z - 1][y][x] ? (chunk->i[z - 1][y][x] &= ~POSITIVE_Z) : (chunk->i[z][y][x] |= NEGATIVE_Z);
    else chunk->i[z][y][x] |= NEGATIVE_Z;

    chunk->i[z][y][x] |= NOT_EMPTY;
}

//TODO: check chunk barrier faces
void remove_block(Chunk *chunk, u8 x, u8 y, u16 z)
{
    x %= CHUNK_SIZE;
    y %= CHUNK_SIZE;

    if (x < CHUNK_SIZE - 1)
        chunk->i[z][y][x + 1] ? (chunk->i[z][y][x + 1] |= NEGATIVE_X) : 0;

    if (x > 0)
        chunk->i[z][y][x - 1] ? (chunk->i[z][y][x - 1] |= POSITIVE_X) : 0;

    if (y < CHUNK_SIZE - 1)
        chunk->i[z][y + 1][x] ? (chunk->i[z][y + 1][x] |= NEGATIVE_Y) : 0;

    if (y > 0)
        chunk->i[z][y - 1][x] ? (chunk->i[z][y - 1][x] |= POSITIVE_Y) : 0;

    if (z < worldHeight - 1)
        chunk->i[z + 1][y][x] ? (chunk->i[z + 1][y][x] |= NEGATIVE_Z) : 0;
    
    if (z > 0)
        chunk->i[z - 1][y][x] ? (chunk->i[z - 1][y][x] |= POSITIVE_Z) : 0;

    chunk->i[z][y][x] = 0;
}

void parse_chunk_states(Chunk *chunk, u16 height)
{
    if (chunk->state & STATE_CHUNK_LOADED) return;
    for (u16 z = 0; z < height; ++z)
        for (u8 y = 0; y < CHUNK_SIZE; ++y)
            for (u8 x = 0; x < CHUNK_SIZE; ++x)
            {
                add_block(chunk, x, y, z);
                if (chunk->i[z][y][x])
                    ++blockCount;
                if (chunk->i[z][y][x] & POSITIVE_X)
                    ++quadCount;
                if (chunk->i[z][y][x] & NEGATIVE_X)
                    ++quadCount;
                if (chunk->i[z][y][x] & POSITIVE_Y)
                    ++quadCount;
                if (chunk->i[z][y][x] & NEGATIVE_Y)
                    ++quadCount;
                if (chunk->i[z][y][x] & POSITIVE_Z)
                    ++quadCount;
                if (chunk->i[z][y][x] & NEGATIVE_Z)
                    ++quadCount;
            }
    chunk->state |= STATE_CHUNK_LOADED;
}

//TODO: revise, might not be needed
Chunk *get_chunk(v3i32 *coordinates, u16 *state, u16 flag)
{
    for (u8 y = 0; y < setting.renderDistance*2; ++y)
    {
        for (u8 x = 0; x < setting.renderDistance*2; ++x)
        {
            if (!(chunkBuf[y][x].state & STATE_CHUNK_LOADED))
            {
                *state &= ~flag;
                return NULL;
            }

            if (
                    chunkBuf[y][x].pos.x == (i32)floorf((f32)coordinates->x/CHUNK_SIZE) &&
                    chunkBuf[y][x].pos.y == (i32)floorf((f32)coordinates->y/CHUNK_SIZE))
            {
                *state |= flag;
                return &chunkBuf[y][x];
            }
        }
    }
    return NULL;
}

Texture2D cobblestone;
Texture2D dirt;
void draw_chunk_buffer(Chunk *chunkBuf)
{
    if (state & STATE_DEBUG_MORE)
        opacity = 200;
    else opacity = 255;
    rlPushMatrix();
    rlSetTexture(dirt.id); //temp texturing
    rlBegin(RL_QUADS);

    for (u16 i = 0; i < sqr((SETTING_RENDER_DISTANCE_MAX*2) + 1); ++i)
        if (chunkBuf[i].state & STATE_CHUNK_LOADED)
            draw_chunk(&chunkBuf[i], 0);

    rlEnd();
    rlPopMatrix();
    rlSetTexture(0); //temp texturing
}

void draw_chunk(Chunk *chunk, u16 height)
{
    if (!height) height = 30;
    rlTranslatef(chunk->pos.x*CHUNK_SIZE, chunk->pos.y*CHUNK_SIZE, WORLD_BOTTOM);

    u16 z = 0; u8 y = 0, x = 0;
    for (; z < height; ++z)
    {
        for (; y < CHUNK_SIZE; ++y)
        {
            for (; x < CHUNK_SIZE; ++x)
            {
                if (chunk->i[z][y][x] & BLOCKFACES)
                    draw_block(chunk->i[z][y][x]);
                rlTranslatef(1, 0, 0);
            }
            x = 0;
            rlTranslatef(-CHUNK_SIZE, 1, 0);
        }
        y = 0;
        rlTranslatef(0, -CHUNK_SIZE, 1);
    }
    rlTranslatef(
            (-chunk->pos.x*CHUNK_SIZE),
            (-chunk->pos.y*CHUNK_SIZE),
            (-WORLD_BOTTOM) - height
            );
}

// raylib/rmodels.c/DrawCube refactored
void draw_block(u32 block_state)
{
    if (block_state & POSITIVE_X)
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

    if (block_state & NEGATIVE_X)
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

    if (block_state & POSITIVE_Y)
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

    if (block_state & NEGATIVE_Y)
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

    if (block_state & POSITIVE_Z)
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

    if (block_state & NEGATIVE_Z)
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

// raylib/rmodels.c/DrawCubeWires refactored
void draw_block_wires(v3i32 *pos)
{
    rlPushMatrix();
    rlTranslatef(pos->x, pos->y, pos->z);
    rlBegin(RL_LINES);
    rlColor4ub(0, 0, 0, 150);

    rlVertex3f(0, 0, 1);
    rlVertex3f(1, 0, 1);

    rlVertex3f(1, 0, 1);
    rlVertex3f(1, 1, 1);

    rlVertex3f(1, 1, 1);
    rlVertex3f(0, 1, 1);

    rlVertex3f(0, 1, 1);
    rlVertex3f(0, 0, 1);

    rlVertex3f(0, 0, 0);
    rlVertex3f(1, 0, 0);

    rlVertex3f(1, 0, 0);
    rlVertex3f(1, 1, 0);

    rlVertex3f(1, 1, 0);
    rlVertex3f(0, 1, 0);

    rlVertex3f(0, 1, 0);
    rlVertex3f(0, 0, 0);

    rlVertex3f(0, 0, 1);
    rlVertex3f(0, 0, 0);

    rlVertex3f(1, 0, 1);
    rlVertex3f(1, 0, 0);

    rlVertex3f(1, 1, 1);
    rlVertex3f(1, 1, 0);

    rlVertex3f(0, 1, 1);
    rlVertex3f(0, 1, 0);

    rlEnd();
    rlPopMatrix();
}

// raylib/rmodels.c/DrawCubeWires refactored
void draw_bounding_box(Vector3 *origin, Vector3 *scl)
{
    rlPushMatrix();
    rlTranslatef(
            origin->x - (scl->x/2),
            origin->y - (scl->y/2),
            origin->z);
    rlBegin(RL_LINES);
    rlColor4ub(255, 200, 220, 255);

    rlVertex3f(0,       0,      scl->z);
    rlVertex3f(scl->x,  0,      scl->z);

    rlVertex3f(scl->x,  0,      scl->z);
    rlVertex3f(scl->x,  scl->y, scl->z);

    rlVertex3f(scl->x,  scl->y, scl->z);
    rlVertex3f(0,       scl->y, scl->z);

    rlVertex3f(0,       scl->y, scl->z);
    rlVertex3f(0,       0,      scl->z);

    rlVertex3f(0,       0,      0);
    rlVertex3f(scl->x,  0,      0);

    rlVertex3f(scl->x,  0,      0);
    rlVertex3f(scl->x,  scl->y, 0);

    rlVertex3f(scl->x,  scl->y, 0);
    rlVertex3f(0,       scl->y, 0);

    rlVertex3f(0,       scl->y, 0);
    rlVertex3f(0,       0,      0);

    rlVertex3f(0,       0,      scl->z);
    rlVertex3f(0,       0,      0);

    rlVertex3f(scl->x,  0,      scl->z);
    rlVertex3f(scl->x,  0,      0);

    rlVertex3f(scl->x,  scl->y, scl->z);
    rlVertex3f(scl->x,  scl->y, 0);

    rlVertex3f(0,       scl->y, scl->z);
    rlVertex3f(0,       scl->y, 0);

    rlEnd();
    rlPopMatrix();
}

void draw_bounding_box_clamped(Vector3 *origin, Vector3 *scl)
{
    Vector3 start =
        (Vector3){
            floorf(origin->x - (scl->x/2.0f)) - 1,
            floorf(origin->y - (scl->y/2.0f)) - 1,
            floorf(origin->z) - 1,
        };

    Vector3 end =
        (Vector3){
            ceilf(scl->x) + 2,
            ceilf(scl->y) + 2,
            ceilf(scl->z) + 2,
        };

    rlPushMatrix();
    rlTranslatef(
            start.x,
            start.y,
            start.z);
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

// raylib/rmodels.c/DrawLine3D refactored
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, Color color)
{
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(pos_0.x, pos_0.y, pos_0.z);
    rlVertex3f(pos_1.x, pos_1.y, pos_1.z);
}
